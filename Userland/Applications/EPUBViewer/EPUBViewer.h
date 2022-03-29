#pragma once

#include <LibGUI/Widget.h>
#include <AK/String.h>

class EPUBViewerWidget final : public GUI::Widget {
    C_OBJECT(EPUBViewerWidget)

public:
    EPUBViewerWidget();

    void load_file(AK::String const& file_path);
};

class EPUBDocument {
public:
    Optional<EPUBDocument> load_document(const AK::String& document);
private:
    EPUBDocument() { }
};
