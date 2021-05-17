#include "GuiClient.h"
#include "MainWindow.h"
#include <gtkmm/application.h>

int GuiClient::Run()
{
    auto app = Gtk::Application::create("org.gtkmm.main");
    MainWindow mainWindow(this);
    return app->run(mainWindow);
}
