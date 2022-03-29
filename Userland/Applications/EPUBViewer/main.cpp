#include <AK/Debug.h>
#include <LibMain/Main.h>
#include <LibCore/System.h>
#include <LibCore/ArgsParser.h>
#include <LibFileSystemAccessClient/Client.h>
#include <LibGUI/Window.h>
#include <LibGUI/Application.h>
#include "EPUBViewer.h"

ErrorOr<int> serenity_main(Main::Arguments arguments)
{
  TRY(Core::System::pledge("stdio"));

  auto app = TRY(GUI::Application::try_create(arguments));
  auto window = GUI::Window::construct();
  window->set_title("EPUB Viewer");
  window->resize(640, 400);

  auto epub_viewer_widget = TRY(window->try_set_main_widget<EPUBViewerWidget>());

  char const* path = nullptr;
  Core::ArgsParser args_parser;
  args_parser.add_positional_argument(path, "The font file for editing.", "file", Core::ArgsParser::Required::Yes);
  args_parser.parse(arguments);

  if ( !path ) {
    dbgln("No file path provided");
    return 1;
  }

  auto response = FileSystemAccessClient::Client::the().try_request_file_read_only_approved(window, arguments.argv[1]);
  if (response.is_error()) {
    return 1;
  }

  epub_viewer_widget->load_file(response.value());

  return 0;
}
