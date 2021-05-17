#pragma once

#include <gtkmm/window.h>
#include <gtkmm/box.h>
#include <gtkmm/label.h>
#include <gtkmm/radiobutton.h>
#include <gtkmm/entry.h>
#include <gtkmm/comboboxtext.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/button.h>

#include "GuiClient.h"
#include "Credentials.h"
#include <string>
#include <cstdint>
#include <map>
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

protected:
    Gtk::Box m_VBoxMain;
    Gtk::Label m_labelRole, m_labelStreamId, m_labelVideoQuality;
    Gtk::RadioButton::Group m_roleGroup;
    Gtk::RadioButton m_radioButtonStreamer, m_radioButtonReceiver;
    Gtk::Entry m_entryStreamId;
    Gtk::ComboBoxText m_comboBoxVideoQuality;
    Gtk::CheckButton m_checkButtonDisableAudio, m_checkButtonDisableVideo, m_checkButtonSaveRawStream;
    Gtk::Button m_buttonChooseFile, m_buttonGo, m_buttonStop;

private:
    GuiClient* m_owner;
    std::shared_ptr<IClient> m_client;
    std::map<std::string, int> m_qualities;
};
