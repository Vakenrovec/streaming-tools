#pragma once

#include <gtkmm/window.h>
#include <gtkmm/menubar.h>
#include <gtkmm/box.h>
#include <gtkmm/label.h>
#include <gtkmm/radiobutton.h>
#include <gtkmm/entry.h>
#include <gtkmm/comboboxtext.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/button.h>
#include <gtkmm/messagedialog.h>
#include <gtkmm/menuitem.h>
#include <gtkmm/imagemenuitem.h>
#include <gtkmm/separatormenuitem.h>
#include <gtkmm/stock.h>
#include "GuiClient.h"
#include <string>
#include <cstdint>
#include <unordered_map>
#include <memory>

class MainWindow: public Gtk::Window
{
public:
    MainWindow(GuiClient* owner);
    ~MainWindow() override {};

protected:
    bool on_key_press_event(GdkEventKey* event) override;
    void OnButtonChooseFileClicked();
    void OnButtonGoClicked();
    void OnButtonStopClicked();
    void OnMenuPlayClicked();
    void OnMenuQuitClicked();
    void OnMenuAboutClicked();

    Gtk::Box m_VBoxMain;
    
    Gtk::MenuBar m_menuBar;
    Gtk::MenuItem m_menuMain; Gtk::Menu m_subMenuMain;
    Gtk::SeparatorMenuItem hline;
    Gtk::ImageMenuItem m_menuPlay, m_menuQuit;
    Gtk::MenuItem m_menuHelp; Gtk::Menu m_subMenuHelp;
    Gtk::ImageMenuItem m_menuAbout;

    Gtk::Label m_labelRole, m_labelStreamId, m_labelVideoQuality;
    Gtk::RadioButton::Group m_roleGroup;
    Gtk::RadioButton m_radioButtonStreamer, m_radioButtonReceiver;
    Gtk::Entry m_entryStreamId;
    Gtk::ComboBoxText m_comboBoxVideoQuality;
    Gtk::CheckButton m_checkButtonDisableAudio, m_checkButtonDisableVideo, m_checkButtonSaveRawStream;
    Gtk::Button m_buttonChooseFile, m_buttonGo, m_buttonStop;

    Gtk::Image m_image;

private:
    void AddMenu();
    void AddControls();

    GuiClient* m_owner;
    std::shared_ptr<IClient> m_client;
    std::unordered_map<std::string, int> m_qualities;
};
