#include <AK/Error.h>
#include <AK/LexicalPath.h>
#include <AK/RefPtr.h>
#include <AK/String.h>
#include <LibArchive/Zip.h>
#include <LibCompress/Deflate.h>
#include <LibCore/DirIterator.h>
#include <LibCore/File.h>
#include <LibCore/MappedFile.h>
#include <LibCore/System.h>

#include "EPUBViewer.h"


/// cr
static ErrorOr<void> make_path(AK::StringView path) {
    AK::LexicalPath lexed_path(path);
    auto parts = lexed_path.parts_view();
    bool is_folder_path = path.ends_with('/');
    auto parts_size = is_folder_path ? parts.size() : parts.size() - 1;


    AK::LexicalPath folder_path(parts[0]);
    for ( size_t index = 1; index <= parts_size; ++index ) {
        auto mkdir_error = Core::System::mkdir(folder_path.string(), 0755);
        if ( mkdir_error.is_error() && mkdir_error.error().code() != EEXIST) {
            return mkdir_error;
        }
        folder_path = folder_path.append(parts[index]);
    }
    if ( !is_folder_path ) {
        auto file_path = folder_path.append(parts[parts_size - 1]);
        auto file = Core::File::construct(file_path);
        if ( file->error() < 0) {
            return Error::from_errno(file->error());
        }
    }
}

static ErrorOr<void> unpack_zip_member(String const& root_path, Archive::ZipMember zip_member) {
     dbgln(" [*] Member: {} is_dir: {}", zip_member.name, zip_member.is_directory ? "YES"sv : "NO"sv);
     if ( zip_member.is_directory ) {
        if (mkdir(zip_member.name.characters(), 0755) < 0) {
            return Error::from_string_literal("mkdir");
        }
        outln(" extracting: {}", zip_member.name);
        return { };
    }
    auto file_path = String::formatted("{}/{}", root_path , zip_member.name);
    auto make_path_err = make_path(file_path);
    if (make_path_err.is_error()) {
        warnln("Can't write file {}", zip_member.name);
        return make_path_err;
    }
    auto new_file = TRY(Core::File::open(file_path, Core::OpenMode::WriteOnly));

    // TODO: verify CRC32s match!
    switch (zip_member.compression_method) {
    case Archive::ZipCompressionMethod::Store: {
        if (!new_file->write(zip_member.compressed_data.data(), zip_member.compressed_data.size())) {
            warnln("Can't write file contents in {}: {}", zip_member.name, new_file->error_string());
            return Error::from_errno(new_file->error());
        }
        break;
    }
    case Archive::ZipCompressionMethod::Deflate: {
        auto decompressed_data = Compress::DeflateDecompressor::decompress_all(zip_member.compressed_data);
        if (!decompressed_data.has_value()) {
            warnln("Failed decompressing file {}", zip_member.name);
            return Error::from_errno(EFAULT);
        }
        if (decompressed_data.value().size() != zip_member.uncompressed_size) {
            warnln("Failed decompressing file {}", zip_member.name);
            return Error::from_errno(EFAULT);
        }
        if (!new_file->write(decompressed_data.value().data(), decompressed_data.value().size())) {
            warnln("Can't write file contents in {}: {}", zip_member.name, new_file->error_string());
            return Error::from_errno(EFAULT);
        }
        break;
    }
    default:
        VERIFY_NOT_REACHED();
    }

    if (!new_file->close()) {
        warnln("Can't close file {}: {}", zip_member.name, new_file->error_string());
        return Error::from_errno(new_file->error());
    }
}

static ErrorOr<AK::String> unpack_zip_file(const String& epub) {
    struct stat st = TRY(Core::System::stat(epub.view()));

    RefPtr<Core::MappedFile> mapped_file;
    ReadonlyBytes input_bytes;
    if ( st.st_size > 0 ) {
        mapped_file = TRY(Core::MappedFile::map(epub));
        input_bytes = mapped_file->bytes();
    }

    LexicalPath abs_path { epub };

    String tmp_path = String::formatted("/tmp/EPUBViewer/{}/", abs_path.basename());
	

    TRY(make_path(tmp_path));
    TRY(Core::System::chdir(tmp_path));

    dbgln("[EPUBVIEWER] Prepared tmp folder: {}", tmp_path);

    auto zip_file = Archive::Zip::try_create(input_bytes);
    if ( !zip_file.has_value() ) {
        warnln("[EPUBViewer] NOT an EPUB FIle!");
        return Error::from_string_literal("Not a EPUB file");
    }
    zip_file->for_each_member([&](auto zip_member) {
        auto res_or_err = unpack_zip_member(tmp_path, zip_member);
        if (res_or_err.is_error()) {
            return IterationDecision::Break;
        }
        return IterationDecision::Continue;
    });

    return tmp_path;
}

ErrorOr<EPUBDocument> EPUBDocument::try_load_file(String const& path) {
    EPUBDocument result;
    dbgln("[EPUBVIEWER] Start try load file: {}", path);
    AK::String tmp_dir = TRY(unpack_zip_file(path));

    auto iter = Core::DirIterator(path, Core::DirIterator::Flags::SkipDots);

    bool opf_found = false;
    bool ncx_found = false;

    while ( iter.has_next() ) {
        auto current_file = iter.next_path();
        if (current_file.ends_with(".opf") && !opf_found) {
            warnln("[EpubViewer] found OPF file: {}", current_file);
            opf_found = true;
        } else if ( current_file.ends_with(".ncx") && !ncx_found ) {
            warnln("[EpubViewer] found NCX file: {}", current_file);
            ncx_found = true;
        }
    }
    return result;
}

