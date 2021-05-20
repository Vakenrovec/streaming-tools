#include "MainWindow.h"
#include "IClient.h"
#include "Streamer.h"
#include "Receiver.h"
#include "Logger.h"
#include "FileUtils.h"
#include "../credentials-lib/MediaCredentials.h"
#include "../credentials-lib/AboutProgramCredentials.h"
#include "pipeline/PipelineHelper.h"
#include "utility/QueueDataProcessor.h"
#include "PlayableDataProcessor.h"
#include <gtkmm/filechooserdialog.h>
#include <gtkmm/messagedialog.h>
#include <string>
#include <SDL2/SDL.h>

#define MSGDIALOG(caption, info)    Gtk::MessageDialog dialog(caption); \
                                            dialog.set_secondary_text(info); \
                                            dialog.run();
#define MSGDIALOG_EX_INFO(info)             Gtk::MessageDialog dialog("INFO"); \
                                            dialog.set_secondary_text(info); \
                                            dialog.run();
#define MSGDIALOG_EX_WARN(warn)             Gtk::MessageDialog dialog("WARNING"); \
                                            dialog.set_secondary_text(warn); \
                                            dialog.run();
#define MSGDIALOG_EX_ERROR(error)           Gtk::MessageDialog dialog("ERROR"); \
                                            dialog.set_secondary_text(error); \
                                            dialog.run();

MainWindow::MainWindow(GuiClient* owner)
: m_owner(owner) 
, m_VBoxMain(Gtk::ORIENTATION_VERTICAL, 10)
, m_labelRole("Choose your role")
, m_radioButtonReceiver(m_roleGroup, "Receiver")
, m_radioButtonStreamer(m_roleGroup, "Streamer")
, m_labelStreamId("Stream id")
, m_labelVideoQuality("Video quality")
, m_checkButtonDisableAudio("Disable audio")
, m_checkButtonDisableVideo("Disable video")
, m_checkButtonSaveRawStream("Save stream")
, m_buttonChooseFile("Choose file")
, m_buttonGo("Go")
, m_buttonStop("Stop")
, m_client(nullptr)
{
    m_qualities = {
        { "Low",    MediaCredentials::bitrate / 100 },
        { "Middle", MediaCredentials::bitrate / 10 },
        { "High",   MediaCredentials::bitrate }
    };

    this->set_title(AboutProgramCredentials::programName);
    this->set_position(Gtk::WIN_POS_CENTER);
    this->add(m_VBoxMain);
    AddMenu();
    AddControls();
    this->show_all_children();
}

void MainWindow::AddMenu()
{
    m_VBoxMain.pack_start(m_menuBar, Gtk::PACK_SHRINK);

    m_menuBar.append(m_menuMain);
    m_menuMain.set_label("Main");
    m_menuMain.set_submenu(m_subMenuMain);
    m_menuPlay = Gtk::ImageMenuItem(Gtk::Stock::MEDIA_PLAY);
    m_menuPlay.signal_activate().connect(sigc::mem_fun(*this, &MainWindow::OnMenuPlayClicked));
    m_subMenuMain.append(m_menuPlay);
    m_subMenuMain.append(hline);
    m_menuQuit = Gtk::ImageMenuItem(Gtk::Stock::QUIT);
    m_menuQuit.signal_activate().connect(sigc::mem_fun(*this, &MainWindow::OnMenuQuitClicked));
    m_subMenuMain.append(m_menuQuit);

    m_menuBar.append(m_menuHelp);
    m_menuHelp.set_label("Help");
    m_menuHelp.set_submenu(m_subMenuHelp);
    m_menuAbout = Gtk::ImageMenuItem(Gtk::Stock::ABOUT);
    m_menuAbout.signal_activate().connect(sigc::mem_fun(*this, &MainWindow::OnMenuAboutClicked));
    m_subMenuHelp.append(m_menuAbout);
}

void MainWindow::AddControls()
{
    m_image = Gtk::Image(MediaCredentials::imagePath);
    m_VBoxMain.pack_start(m_image);

    m_VBoxMain.pack_start(m_labelRole, Gtk::PACK_SHRINK);
    m_VBoxMain.pack_start(m_radioButtonStreamer);
    m_VBoxMain.pack_start(m_radioButtonReceiver);
    m_radioButtonStreamer.set_active();

    m_VBoxMain.pack_start(m_labelStreamId);
    m_VBoxMain.pack_start(m_entryStreamId, Gtk::PACK_SHRINK);
    m_entryStreamId.set_text(std::to_string(m_owner->m_streamId));

    m_VBoxMain.pack_start(m_labelVideoQuality, Gtk::PACK_SHRINK);
    m_VBoxMain.pack_start(m_comboBoxVideoQuality, Gtk::PACK_SHRINK);
    for (const auto& quality : m_qualities)
    {
        m_comboBoxVideoQuality.append(quality.first);
    }
    m_comboBoxVideoQuality.set_active(0);

    m_VBoxMain.pack_start(m_checkButtonDisableAudio);
    m_VBoxMain.pack_start(m_checkButtonDisableVideo);

    m_VBoxMain.pack_start(m_checkButtonSaveRawStream);

    m_VBoxMain.pack_start(m_buttonChooseFile, Gtk::PACK_SHRINK);
    m_buttonChooseFile.signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::OnButtonChooseFileClicked));

    m_VBoxMain.pack_start(m_buttonGo, Gtk::PACK_SHRINK);
    m_buttonGo.signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::OnButtonGoClicked));

    m_VBoxMain.pack_start(m_buttonStop, Gtk::PACK_SHRINK);
    m_buttonStop.signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::OnButtonStopClicked));
}

bool MainWindow::on_key_press_event(GdkEventKey* key_event)
{
    if(key_event->keyval == GDK_KEY_Escape)
    {
        hide();
        return true;
    }
    return Gtk::Window::on_key_press_event(key_event);
}

void MainWindow::OnButtonChooseFileClicked()
{
    Gtk::FileChooserDialog dialog(
        "Please choose a file",
        Gtk::FILE_CHOOSER_ACTION_SAVE, 
        Gtk::DialogFlags::DIALOG_MODAL
    );
    dialog.set_transient_for(*this);
    dialog.add_button("Cancel", Gtk::RESPONSE_CANCEL);
    dialog.add_button("Select", Gtk::RESPONSE_OK);

    int result = dialog.run();
    
    std::string folder, filename;
    switch (result)
    {
        case (Gtk::RESPONSE_OK):
        {   
            folder = dialog.get_current_folder();
            filename = dialog.get_current_name();
            m_owner->SetRawStreamDir(folder);
            m_owner->SetRawStreamFilename(filename);
            LOG_EX_INFO("Select clicked. Folder: %s, Filename: %s", folder.c_str(), filename.c_str());
            MSGDIALOG_EX_INFO("You selected " + FileUtils::CombinePath(folder, filename));
            break;
        }
        case (Gtk::RESPONSE_CANCEL):
        {
            LOG_EX_INFO("Cancel clicked");
            break;
        }
        default:
        {
            LOG_EX_WARN("Unexpected button clicked");
            break;
        }
    }
}

void MainWindow::OnButtonGoClicked()
{
    if (m_client)
    {
        MSGDIALOG_EX_WARN("Client was already started");
        return;
    }

    std::string strStreamId = m_entryStreamId.get_text().c_str();
    char* pEnd;
    std::uint32_t streamId = std::strtoul(strStreamId.c_str(), &pEnd, 10);    
    if (*pEnd != 0 || strStreamId.size() > 9)
    {
        MSGDIALOG_EX_WARN("Stream id must be a number with less than 10 digits");
        return;
    }
    m_owner->SetStreamId(streamId);
    m_owner->SetBitrate(m_qualities[m_comboBoxVideoQuality.get_active_text()]);
    m_owner->SetDisableAudio(m_checkButtonDisableAudio.get_active());
    m_owner->SetDisableVideo(m_checkButtonDisableVideo.get_active());
    m_owner->SetSaveRawStream(m_checkButtonSaveRawStream.get_active());

    m_client = nullptr;
    if (m_radioButtonStreamer.get_active())
    {
        m_client = std::make_shared<Streamer>();
        LOG_EX_INFO("Streamer was selected");
    }
    else if (m_radioButtonReceiver.get_active())
    {
        m_client = std::make_shared<Receiver>();
        LOG_EX_INFO("Receiver was selected");
    }
    else
    {
        LOG_EX_WARN("Neither receiver nor streamer were selected");
        return;
    }

    m_client->SetServerIp(m_owner->m_serverIp);
    m_client->SetServerTcpPort(m_owner->m_serverTcpPort);
    m_client->SetLocalIp(m_owner->m_localIp);
    m_client->SetLocalUdpPort(m_owner->m_localUdpPort);

    m_client->SetStreamId(m_owner->m_streamId);
    m_client->SetWidth(m_owner->m_width);
    m_client->SetHeight(m_owner->m_height);
    m_client->SetBitrate(m_owner->m_bitrate);
    m_client->SetGopSize(m_owner->m_gopSize);

    m_client->SetDisableAudio(m_owner->m_disableAudio);
    m_client->SetDisableVideo(m_owner->m_disableVideo);

    m_client->SetSaveRawStream(m_owner->m_saveRawStream);
    m_client->SetRawStreamDir(m_owner->m_rawStreamDir);
    m_client->SetRawStreamFilename(m_owner->m_rawStreamFilename);

    m_client->StartAsync();
}

void MainWindow::OnButtonStopClicked()
{
    if (!m_client)
    {
        MSGDIALOG_EX_WARN("Client wasn't started");
        return;
    }

    SDL_Event sdlevent;
    sdlevent.type = SDL_QUIT;
    SDL_PushEvent(&sdlevent);

    m_client->HandleEvents();
    m_client->Destroy();
    m_client = nullptr;
}

void MainWindow::OnMenuPlayClicked()
{
    const std::string& rawStreamDir = m_owner->m_rawStreamDir;
    const std::string& rawStreamFilename = m_owner->m_rawStreamFilename;
    const int width = m_owner->m_width;
    const int height = m_owner->m_height;

    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS);

    Pipeline pipeline = PipelineHelper::CreateMediaplayer(rawStreamDir, rawStreamFilename, width, height);
    auto reader = std::dynamic_pointer_cast<PlayableDataProcessor>((*pipeline)[0]);
    auto audioQueue = std::dynamic_pointer_cast<QueueDataProcessor<udp_packet_ptr>>((*pipeline)[2]);
    auto videoQueue = std::dynamic_pointer_cast<QueueDataProcessor<udp_packet_ptr>>((*pipeline)[8]);

    bool end = false;
    reader->Init();
    if (reader->GetState() != DataProcessor::State::INITIALIZED)
    {
        MSGDIALOG_EX_WARN("Couldn't initialize mediaplayer");
        return;
    }

    std::thread mediaplayerThread([reader, audioQueue, videoQueue, &end](){
        reader->Play();
        LOG_EX_INFO("Everything was read");
        audioQueue->Process(nullptr);
        audioQueue->SetState(DataProcessor::State::STOPPING);
        videoQueue->Process(nullptr);
        videoQueue->SetState(DataProcessor::State::STOPPING); 
        reader->Destroy();
        end = true;
    });    
    SDL_Event e;
    std::chrono::milliseconds delay = std::chrono::seconds(1);
    LOG_EX_INFO("%d", delay);
    for (;;)
    {
        int res = SDL_WaitEventTimeout(&e, delay.count());
        if (end)
        {
            break;
        }
        if (e.type == SDL_QUIT)
        {
            reader->Stop();
            videoQueue->Clear();
            audioQueue->Clear();                
            LOG_EX_INFO("Mediaplayer stopped");
            break;
        }            
    }
    mediaplayerThread.join();

    SDL_Quit();
    LOG_EX_INFO_WITH_CONTEXT("Mediaplayer destroyed");
}

void MainWindow::OnMenuQuitClicked()
{
    hide();
}

void MainWindow::OnMenuAboutClicked()
{
    std::string caption = AboutProgramCredentials::programName;
    std::string about;
    about.reserve(1024); 
    about += "Version: " + AboutProgramCredentials::programVersion + "\n";
    about += "Datetime: " + AboutProgramCredentials::date + " " + AboutProgramCredentials::time + "\n";
    about += "c++: " + std::to_string(AboutProgramCredentials::cppVersion) + "\n";
    about += "GNU: " + AboutProgramCredentials::compilerVersion + "\n";
    about += "OS: " + AboutProgramCredentials::os + "\n";
    
    MSGDIALOG(caption, about);
}
