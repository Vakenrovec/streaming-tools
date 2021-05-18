#include "GuiClient.h"
#include "MainWindow.h"
#include <gtkmm/application.h>
#include <random>
#include <string>

int GuiClient::Run()
{
    std::random_device randomDevice;
    std::mt19937 generator(randomDevice());
    auto app = Gtk::Application::create("org.gtkmm.main-" + std::to_string(generator()));
    MainWindow mainWindow(this);
    return app->run(mainWindow);
}
