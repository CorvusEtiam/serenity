serenity_component(
    EPUBViewer
    RECOMMENDED
    TARGETS EPUBViewer
)

# compile_gml(HexEditorWindow.gml HexEditorWindowGML.h hex_editor_window_gml)
# compile_gml(GoToOffsetDialog.gml GoToOffsetDialogGML.h go_to_offset_dialog_gml)
# compile_gml(FindDialog.gml FindDialogGML.h find_dialog_gml)

# compile_gml(EPUBViewerWindow.gml EPUBVieweWindowGML.h epub_viewer_window_gml)

set(SOURCES
    EPUBViewer.h
    EPUBViewer.cpp
    main.cpp
)

serenity_app(EPUBViewer ICON app-hex-editor)
target_link_libraries(EPUBViewer LibGUI LibMain LibFileSystemAccessClient)
