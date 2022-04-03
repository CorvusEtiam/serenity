#pragma once
#include <AK/Error.h>
#include <AK/String.h>
#include <AK/HashMap.h>


struct NavMapItem {
    int id { 0 };
    AK::String label;
    AK::String target;
};

struct Chapter {
    AK::String html_content;
};


class EPUBDocument {
public:
    static ErrorOr<EPUBDocument> try_load_file(String const& path);

    AK::HashMap<AK::String, NavMapItem>& toc() { return m_toc; }
    AK::HashMap<AK::String, AK::String>& metadata() { return m_metadata; }
    AK::HashMap<AK::String, Chapter>& chapters() { return m_chapters; }

    AK::HashMap<AK::String, NavMapItem>const & toc() const { return m_toc; }
    AK::HashMap<AK::String, AK::String>const& metadata() const { return m_metadata; }
    AK::HashMap<AK::String, Chapter>const& chapters() const { return m_chapters; }

    void set_title(AK::String title) { m_title = title; }
    AK::StringView title() { return m_title; }

private:
    EPUBDocument() { }
    AK::String m_title;
    AK::HashMap<AK::String, NavMapItem> m_toc;
    AK::HashMap<AK::String, AK::String> m_metadata;
    AK::HashMap<AK::String, Chapter>    m_chapters;
};
