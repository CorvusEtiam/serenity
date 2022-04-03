#include <AK/String.h>
#include <LibMain/Main.h>
#include <LibCore/System.h>
#include <LibCore/ArgsParser.h>

#include "EPUBViewer.h"

ErrorOr<int> serenity_main(Main::Arguments arguments)
{
  TRY(Core::System::pledge("stdio rpath cpath wpath"));

  char const* path = nullptr;
  Core::ArgsParser args_parser;
  args_parser.add_positional_argument(path, "The epub file", "file", Core::ArgsParser::Required::Yes);
  args_parser.parse(arguments);

  String epub_path { path };

  TRY(EPUBDocument::try_load_file(epub_path));

  return 0;
}
