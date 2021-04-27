#include <hardt.h>

#include <iomanip>
#include <chrono>
#include <thread>
#include <regex>

#include "main.h"
#include "booma.h"
#include "mainwindow.h"
#include "inputdialog.h"

bool MainWindow::_threadsRunning = true;
int MainWindow::_threadsAlive = 0;

/***********************************************
  Static callbacks
***********************************************/

MainWindow* MainWindow::_instance;

/**
 * Static callback for menu handling
 * @param w The menubar widget
 * @param data Pointer to the mainwindow
 */
void HandleMenuButtonCallback(Fl_Widget* w, void* data) {
    char name[80];
    ((Fl_Menu_Button*) w)->item_pathname(name, sizeof(name)-1);
    MainWindow::Instance()->HandleMenuButton(name);
}

/**
 * Static callback for frequency input buttons
 * @param w Calling button
 * @param data (Unused)
 */
void HandleFrequencyInputButtonsCallback(Fl_Widget *w) {
    MainWindow::Instance()->HandleFrequencyInputButtons(w);
}

/**
 * Static callback for frequency input events
 * @param w Calling input field
 * @param data (Unused)
 */
void HandleFrequencyInputCallback(Fl_Widget *w) {
    MainWindow::Instance()->HandleFrequencyInput(w);
}

/**
 * Static callback for channel selection events
 * @param w
 */
void HandleChannelSelectorCallback(Fl_Widget* w) {
    MainWindow::Instance()->HandleChannelSelector(w);
}

/**
 * Static callback for gain slider events
 * @param w
 */
void HandleGainSliderCallback(Fl_Widget* w) {
    MainWindow::Instance()->HandleGainSlider();
}

/**
 * Static callback for gain-enabled button events
 * @param w
 */
void HandleGainEnabledCallback(Fl_Widget* w) {
    MainWindow::Instance()->HandleGainEnabled();
}

/**
 * Static callback for gain slider events
 * @param w
 */
void HandleVolumeSliderCallback(Fl_Widget* w) {
    MainWindow::Instance()->HandleVolumeSlider();
}

static void HandleMainWindowExit(Fl_Widget *widget, void *)
{
    MainWindow* window = (MainWindow*) widget;
    window->Exit();
}

/**
 * Static callback for offset input event
 * @param w Calling input field
 * @param data (Unused)
 */
void HandleFrequencyOffsetCallback(Fl_Widget *w) {
    MainWindow::Instance()->HandleFrequencyOffset(w);
}

/**
 * Static callback for menu handling
 * @param w The menubar widget
 * @param data Pointer to the mainwindow
 */
void HandleRfWaterfallCallback(Fl_Widget* w) {
    MainWindow::Instance()->HandleRfWaterfall();
}

void HandleFrequencyNavigationCallback(Fl_Widget* w, void* data) {
    char name[80];
    ((Fl_Menu_Button*) w)->item_pathname(name, sizeof(name)-1);
    MainWindow::Instance()->HandleFrequencyNavigation(name);
}

int HandleAllEvents(int event) {
    if (event == FL_SHORTCUT) {
        if( Fl::event_ctrl() && Fl::event_key() == 'f' ) {
            MainWindow::Instance()->HandleCtrlF();
            return 1;
        }
        if( Fl::event_ctrl() && Fl::event_key() == 'o' ) {
            MainWindow::Instance()->HandleCtrlO();
            return 1;
        }
        if( Fl::event_key() == FL_Escape ) {
            MainWindow::Instance()->HandleEscape();
            return 1;
        }
    }
    return 0;
}

void HandleMenuCtrlF(Fl_Widget* w, void* data) {
    HandleAllEvents(FL_SHORTCUT);
}


void HandleMenuCtrlO(Fl_Widget* w, void* data) {
    HandleAllEvents(FL_SHORTCUT);
}

/***********************************************
  Member functions
***********************************************/

/**
 * Construct the mainwindow
 * @param app Initialized BoomaApplication
 */
MainWindow::MainWindow(BoomaApplication* app):
    _app(app) {

    // Save current instance
    _instance = this;

    // Create the window
    _win = new Fl_Window(720, 486, GetTitle());
    _win->callback(HandleMainWindowExit);

    // Compose gui
    SetupMenus();
    SetupControls();
    SetupDisplays();
    SetupStatusbar();
    _win->end();
    _win->show();

    // Start display threads
    Fl::lock();
    _signalLevelThread = new std::thread( [this]()  {
        _threadsAlive++;
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        while( MainWindow::_threadsRunning ) {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            if( !_threadsPaused ) {
                Fl::lock();
                UpdateSignalLevelDisplay();
                Fl::unlock();
                Fl::awake();
            }
        }
        _threadsAlive--;
    } );
    _rfSpectrumThread = new std::thread( [this]()  {
        _threadsAlive++;
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        while( _threadsRunning ) {
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            if( !_threadsPaused ) {
                Fl::lock();
                UpdateRfSpectrumDisplay();
                Fl::unlock();
                Fl::awake();
            }
        }
        _threadsAlive--;
    } );
    _afSpectrumThread = new std::thread( [this]()  {
        _threadsAlive++;
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        while( _threadsRunning ) {
            std::this_thread::sleep_for(std::chrono::milliseconds(150));
            if( !_threadsPaused ) {
                Fl::lock();
                UpdateAfSpectrumDisplay();
                Fl::unlock();
                Fl::awake();
            }
        }
        _threadsAlive--;
    } );
    _halterThread = new std::thread([this]() {
        while( _threadsRunning || _threadsAlive > 0 ) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        HLog("No more running threads, closing mainwindow");
        _win->hide();
        Fl::awake();
    });

    // Add a global event handler to handle general keypresses without an menu entry
    Fl::add_handler(HandleAllEvents);

    // Set initial focus
    Fl::focus(_win);

    // Start the receiver
    Run();
}

/**
 * Destruct the mainwindow
 */
MainWindow::~MainWindow() {
    HLog("Destroying the MainWindow");

    // Cleanup
    delete _frequencyInput;
    delete _frequencyInputSet;
    delete _frequencyInputUp100;
    delete _frequencyInputDown100;
    delete _frequencyInputUp1Khz;
    delete _frequencyInputDown1Khz;
    delete _channelSelector;
    delete _gainSlider;
    delete _volumeSlider;
    delete _rfInputWaterfall;
    delete _afOutputWaterfall;
    delete _signalLevelSlider;

    // Join threads
    _signalLevelThread->join();
    _rfSpectrumThread->join();
    _afSpectrumThread->join();
    _halterThread->join();
}

/**
 * Stop receiver and exit the mainwindow - effectively closing the application
 */
void MainWindow::Exit() {
    HLog("Halting threads and receiver");
    MainWindow::_threadsRunning = false;
}

/**
 * Get the application title
 * @return Pointer to a static string with the title
 */
char* MainWindow::GetTitle() {
    static char title[256];

    std::stringstream ss;
    ss << "Booma " << BOOMAGUI_MAJORVERSION << "." << BOOMAGUI_MINORVERSION << "." << BOOMAGUI_BUILDNO;
    ss << "   (";
    ss << "libbooma " << BOOMA_MAJORVERSION << "." << BOOMA_MINORVERSION << "." << BOOMA_BUILDNO;
    ss << ", ";
    ss << " Hardt " << getVersion();
    ss << ")";
    memcpy(title, ss.str().c_str(), ss.str().length());
    return title;
}


/***********************************************
  Gui initialization
***********************************************/

/**
 * Create the menubar
 */
void MainWindow::SetupMenus() {
    _menubar = new Fl_Menu_Bar(0, 0, _win->w(), 25);

    // File
    SetupFileMenu();

    // Configuration
    SetupConfigurationMenu();

    // Navigation
    SetupNavigationMenu();

    // Receiver
    SetupReceiverMenu();
    SetupReceiverInputMenu();
    SetupReceiverOutputMenu();
    SetupReceiverModeMenu();

    // Options
    SetupSettingsMenu();

    // Help
    _menubar->add("Help/Help", 0, HandleMenuButtonCallback, (void*) this);

    // Make sure the menu state is current
    UpdateState();
}

void MainWindow::SetupFileMenu() {
    _menubar->add("File/Quit", "^q", HandleMenuButtonCallback, (void*) this);
}

void MainWindow::SetupConfigurationMenu() {

    RemoveMenuSubItems("Configuration/Inputs/*");

    std::vector<std::string> configSections = _app->GetConfigSections();
    for( int i = 0; i < configSections.size(); i++ ) {
        _menubar->add(("Configuration/Inputs/Delete " + configSections.at(i)).c_str(), 0, HandleMenuButtonCallback, (void*) this,
                      (_app->GetConfigSection() == configSections.at(i) ? FL_MENU_INACTIVE : 0) |
                      (i == configSections.size() - 1 ? FL_MENU_DIVIDER : 0));
    }

    _menubar->add("Configuration/Inputs/Add input", 0, HandleMenuButtonCallback, (void*) this);
}

void MainWindow::SetupReceiverMenu() {

    RenameMenuItem("Receiver/Stop", "Start");
    RenameMenuItem("Receiver/Stop dumping RF", "Dump RF");
    RenameMenuItem("Receiver/Stop dumping AF", "Dump AF");

    if( _app->IsRunning() ) {
        _menubar->add("Receiver/Stop", "^s", HandleMenuButtonCallback, (void*) this);
    } else {
        _menubar->add("Receiver/Start", "^s", HandleMenuButtonCallback, (void*) this);
    }
    _menubar->add("Receiver/Restart", "^r", HandleMenuButtonCallback, (void*) this, FL_MENU_DIVIDER);

    _menubar->add("Receiver/Dump RF", "^p", HandleMenuButtonCallback, (void*) this);
    _menubar->add("Receiver/Dump AF", "^u", HandleMenuButtonCallback, (void*) this, FL_MENU_DIVIDER);
}

void MainWindow::SetupReceiverInputMenu() {

    RemoveMenuSubItems("Receiver/Input/*");

    std::vector<std::string> configSections = _app->GetConfigSections();
    for( int i = 0; i < configSections.size(); i++ ) {
        _menubar->add(("Receiver/Input/" + configSections.at(i)).c_str(), 0, HandleMenuButtonCallback, (void*) this,
                FL_MENU_RADIO |
                (_app->GetConfigSection() == configSections.at(i) ? FL_MENU_VALUE : 0) |
                (i == configSections.size() - 1 ? FL_MENU_DIVIDER : 0));
    }

    _menubar->add("Receiver/Input/Edit active input", 0, HandleMenuButtonCallback, (void*) this);
}

void MainWindow::SetupReceiverOutputMenu() {

    RemoveMenuSubItems("Receiver/Output/*");

    std::map<int, std::string> hardwareCards = _app->GetAudioDevices(true, false, false, true);
    for( std::map<int, std::string>::iterator it = hardwareCards.begin(); it != hardwareCards.end(); it++) {
        _menubar->add(("Receiver/Output/Card "  + std::to_string((*it).first) + ": " + (*it).second).c_str(), 0, HandleMenuButtonCallback, (void *) this,
                      FL_MENU_RADIO | (_app->GetOutputDevice() == (*it).first ? FL_MENU_VALUE : 0));
    }

    std::map<int, std::string> virtualCards = _app->GetAudioDevices(false, true, false, true);
    for( std::map<int, std::string>::iterator it = virtualCards.begin(); it != virtualCards.end(); it++) {
        _menubar->add(("Receiver/Output/Card "  + std::to_string((*it).first) + ": " + (*it).second).c_str(), 0, HandleMenuButtonCallback, (void *) this,
                      FL_MENU_RADIO | (_app->GetOutputDevice() == (*it).first ? FL_MENU_VALUE : 0));
    }

    _menubar->add("Receiver/Output/Silence", 0, HandleMenuButtonCallback, (void *) this,
                  FL_MENU_RADIO | (_app->GetOutputDevice() == -1 && _app->GetOutputFilename() == "" ? FL_MENU_VALUE : 0));

    _menubar->add("Receiver/Output/File", 0, HandleMenuButtonCallback, (void *) this,
                  FL_MENU_RADIO | (_app->GetOutputFilename() != "" ? FL_MENU_VALUE : 0));
}

void MainWindow::SetupReceiverModeMenu() {
    _menubar->add("Receiver/Mode/AURORAL", 0, HandleMenuButtonCallback, (void*) this,
                  FL_MENU_RADIO | (_app->GetReceiver() == ReceiverModeType::AURORAL ? FL_MENU_VALUE : 0));
    _menubar->add("Receiver/Mode/AM", 0, HandleMenuButtonCallback, (void*) this,
                  FL_MENU_RADIO | (_app->GetReceiver() == ReceiverModeType::AM ? FL_MENU_VALUE : 0));
    _menubar->add("Receiver/Mode/CW", 0, HandleMenuButtonCallback, (void*) this,
                  FL_MENU_RADIO | (_app->GetReceiver() == ReceiverModeType::CW ? FL_MENU_VALUE : 0));
    _menubar->add("Receiver/Mode/SSB", 0, HandleMenuButtonCallback, (void*) this,
                  FL_MENU_RADIO | (_app->GetReceiver() == ReceiverModeType::SSB ? FL_MENU_VALUE : 0));
}

void MainWindow::SetupSettingsMenu() {

    RemoveMenuSubItems("Receiver/Settings/*");

    // Define some usefull defaults for the IF filter
    _menubar->add("Receiver/Settings/IF filter width/500 Hz", 0, HandleMenuButtonCallback, (void*) this,
                  FL_MENU_RADIO | (_app->GetInputFilterWidth() == 500 ? FL_MENU_VALUE : 0));
    _menubar->add("Receiver/Settings/IF filter width/1000 Hz", 0, HandleMenuButtonCallback, (void*) this,
                  FL_MENU_RADIO | (_app->GetInputFilterWidth() == 1000 ? FL_MENU_VALUE : 0));
    _menubar->add("Receiver/Settings/IF filter width/3000 Hz", 0, HandleMenuButtonCallback, (void*) this,
                  FL_MENU_RADIO | (_app->GetInputFilterWidth() == 3000 ? FL_MENU_VALUE : 0));
    _menubar->add("Receiver/Settings/IF filter width/5000 Hz", 0, HandleMenuButtonCallback, (void*) this,
                  FL_MENU_RADIO | (_app->GetInputFilterWidth() == 5000 ? FL_MENU_VALUE : 0));
    _menubar->add("Receiver/Settings/IF filter width/8000 Hz", 0, HandleMenuButtonCallback, (void*) this,
                  FL_MENU_RADIO | (_app->GetInputFilterWidth() == 8000 ? FL_MENU_VALUE : 0));
    _menubar->add("Receiver/Settings/IF filter width/10000 Hz", 0, HandleMenuButtonCallback, (void*) this,
                  FL_MENU_RADIO | (_app->GetInputFilterWidth() == 10000 ? FL_MENU_VALUE : 0));
    _menubar->add("Receiver/Settings/IF filter width/off", 0, HandleMenuButtonCallback, (void*) this,
                  FL_MENU_RADIO | (_app->GetInputFilterWidth() == 0 ? FL_MENU_VALUE : 0));

    // Preamp settings
    _menubar->add("Receiver/Settings/Preamp/-12 dB", 0, HandleMenuButtonCallback, (void*) this,
                  FL_MENU_RADIO | (_app->GetPreampLevel() < 0 ? FL_MENU_VALUE : 0));
    _menubar->add("Receiver/Settings/Preamp/Off", 0, HandleMenuButtonCallback, (void*) this,
                  FL_MENU_RADIO | (_app->GetPreampLevel() == 0 ? FL_MENU_VALUE : 0));
    _menubar->add("Receiver/Settings/Preamp/+12 dB", 0, HandleMenuButtonCallback, (void*) this,
                  FL_MENU_RADIO | (_app->GetPreampLevel() == 1 ? FL_MENU_VALUE : 0));
    _menubar->add("Receiver/Settings/Preamp/+24 dB", 0, HandleMenuButtonCallback, (void*) this,
                  FL_MENU_RADIO | (_app->GetPreampLevel() > 1 ? FL_MENU_VALUE : 0));

    // Otherwise show available options
    for( std::vector<Option>::iterator it = _app->GetOptions()->begin(); it != _app->GetOptions()->end(); it++ ) {
        std::vector<OptionValue> values = (*it).Values;
        for( std::vector<OptionValue>::iterator vit = values.begin(); vit != values.end(); vit++ ) {
            _menubar->add( ("Receiver/Settings/" + (*it).Name + "/" + (*vit).Name).c_str(), 0, HandleMenuButtonCallback, (void*) this,
                           FL_MENU_RADIO | ( (*vit).Value == (*it).CurrentValue ? FL_MENU_VALUE : 0) );
        }
    }
}

void MainWindow::SetupStatusbar(){
    _statusbar = new Fl_Group(0, _win->h() - 20, _win->w(), 20);
    _statusbar->box(FL_FLAT_BOX);
    _statusbar->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
    _statusbar->color(48-2);
    _statusbar->begin();

    _statusbarConfig = new Fl_Output(_statusbar->x(), _statusbar->y(), 180, 20);
    _statusbarConfig->box(FL_THIN_DOWN_FRAME);
    _statusbarConfig->color(FL_GRAY0);

    _statusbarMode = new Fl_Output(_statusbarConfig->x() + _statusbarConfig->w(), _statusbar->y(), 100, 20);
    _statusbarMode->box(FL_THIN_DOWN_FRAME);
    _statusbarMode->color(FL_GRAY0);

    _statusbarHardwareFreq = new Fl_Output(_statusbarMode->x() + _statusbarMode->w(), _statusbar->y(), 100, 20);
    _statusbarHardwareFreq->box(FL_THIN_DOWN_FRAME);
    _statusbarHardwareFreq->color(FL_GRAY0);

    _statusbarRunningState = new Fl_Output(_statusbarHardwareFreq->x() + _statusbarHardwareFreq->w(), _statusbar->y(), 100, 20);
    _statusbarRunningState->box(FL_THIN_DOWN_FRAME);

    _statusbarRecordingRf = new Fl_Output(_statusbarRunningState->x() + _statusbarRunningState->w(), _statusbar->y(), 120, 20);
    _statusbarRecordingRf->box(FL_THIN_DOWN_FRAME);
    _statusbarRecordingRf->color(FL_GRAY0);

    _statusbarRecordingAf = new Fl_Output(_statusbarRecordingRf->x() + _statusbarRecordingRf->w(), _statusbar->y(), 120, 20);
    _statusbarRecordingAf->box(FL_THIN_DOWN_FRAME);
    _statusbarRecordingAf->color(FL_GRAY0);

    _statusbar->end();
    _statusbar->show();
}

/**
 * Setup controls in the main window
 */
void MainWindow::SetupControls() {

    // Frequency input
    _frequencyInput = new Fl_Input(90, _win->h() - 80, 100, 30, "Frequency ");
    _frequencyInput->value(std::to_string(_app->GetFrequency()).c_str());
    _frequencyInput->when(FL_WHEN_ENTER_KEY_ALWAYS);
    _frequencyInputSet = new Fl_Button(200, _win->h() - 80, 50, 30, "set");
    _frequencyInputDown1Khz = new Fl_Button(260, _win->h() - 80, 30, 30, "<<");
    _frequencyInputDown100 = new Fl_Button(290, _win->h() - 80, 30, 30, "<");
    _frequencyInputUp100 = new Fl_Button(325, _win->h() - 80, 30, 30, ">");
    _frequencyInputUp1Khz = new Fl_Button(355, _win->h() - 80, 30, 30, ">>");
    _frequencyOffset = new Fl_Input(395, _win->h() - 80, 50, 30);
    _frequencyOffset->when(FL_WHEN_ENTER_KEY_ALWAYS);
    _frequencyOffset->value(std::to_string(_app->GetOffset()).c_str());
    _frequencyInput->callback(HandleFrequencyInputCallback);
    _frequencyOffset->callback(HandleFrequencyOffsetCallback);
    _frequencyInputSet->callback(HandleFrequencyInputButtonsCallback);
    _frequencyInputUp1Khz->callback(HandleFrequencyInputButtonsCallback);
    _frequencyInputUp100->callback(HandleFrequencyInputButtonsCallback);
    _frequencyInputDown100->callback(HandleFrequencyInputButtonsCallback);
    _frequencyInputDown1Khz->callback(HandleFrequencyInputButtonsCallback);
    _frequencyInput->tooltip("Current frequency");
    _frequencyInputSet->tooltip("Set receiver to the frequency given in the current frequency field (or just press ENTER after editing the frequency)");
    _frequencyOffset->tooltip("Offset from zero for rtlsdr devices");
    _frequencyInputDown100->tooltip("Tune 100Hz lower");
    _frequencyInputDown1Khz->tooltip("Tune 1KHz lower");
    _frequencyInputUp100->tooltip("Tune 100Hz higher");
    _frequencyInputUp1Khz->tooltip("Tune 1KHz higher");

    // Channel selector
    _channelSelector = new Fl_Choice(470, _win->h() - 80, 240, 30);
    _channelSelector->callback(HandleChannelSelectorCallback);
    SetupChannels();

    // Gain slider and agc enable/disable
    _gainSlider = new Fl_Slider(FL_HOR_NICE_SLIDER, _win->w() - 190, _menubar->h() + 10, 120, 30, _gainLabel);
    _gainSlider->bounds(-20, 110);
    _gainSlider->scrollvalue(MapToGainSliderValue(_app->GetRfGain()), 1, -20, 131);
    SetGainSliderLabel();
    _gainSlider->callback(HandleGainSliderCallback);
    _gainEnabled = new Fl_Light_Button(_win->w() - 60, _menubar->h() + 10, 50, 30, "AGC");
    _gainEnabled->value(_app->GetRfGainEnabled() ? 1 : 0);
    _gainEnabled->color(FL_GRAY, FL_GREEN);
    _gainEnabled->callback(HandleGainEnabledCallback);
    if( _app->GetRfGainEnabled() ) {
        _gainSlider->set_active();
    } else {
        _gainSlider->clear_active();
    }

    // Volume slider
    _volumeSlider = new Fl_Slider(FL_HOR_NICE_SLIDER, _win->w() - 190, _menubar->h() + 80, 180, 30, _volumeLabel);
    _volumeSlider->bounds(0, 30);
    _volumeSlider->scrollvalue(_app->GetVolume(), 1, 0, 31);
    SetVolumeSliderLabel();
    _volumeSlider->callback(HandleVolumeSliderCallback);
}

void MainWindow::SetupChannels() {
    _channelSelector->clear();

    std::map<int, Channel*> channels = _app->GetChannels();
    for( std::map<int, Channel*>::iterator it = channels.begin(); it != channels.end(); it++ ) {
        std::string name = (*it).second->Name;
        std::string choiceName = regex_replace(name, std::regex("\\/"), "\\/");
        std::cout << choiceName << std::endl;

        _channelSelector->add((std::to_string((*it).second->Frequency) + "  " + choiceName).c_str());
    }
}

/**
 * Setup display widgets in the main window
 */
void MainWindow::SetupDisplays() {

    // Signallevel reporting
    _signalLevelSlider = new Fl_Slider(FL_HOR_FILL_SLIDER, _win->w() - 190, _menubar->h() + 150, 180, 30, "S0");
    _signalLevelSlider->bounds(0, 11);
    _signalLevelSlider->set_output();
    _signalLevelSlider->scrollvalue(0, 1,0, 12);
    _signalLevelSlider->color(FL_GRAY, FL_GREEN);

    // RF input waterfall
    _rfInputWaterfall = new Waterfall(10, _menubar->h() + 10, 512, 185, "RF input",
                                      _app->GetRfFftSize(),
                                      _app->GetInputSourceDataType() != REAL_INPUT_SOURCE_DATA_TYPE,
                                      _app,
                                      1,
                                      _app->GetOutputSampleRate() / 2,
                                      RF);
    _rfInputWaterfall->callback(HandleRfWaterfallCallback);

    // AF output waterfall
    _afOutputWaterfall = new Waterfall(10, _rfInputWaterfall->y() + _rfInputWaterfall->h() + 10, 128, 140, "AF output",
                                       _app->GetAudioFftSize(),
                                       false,
                                       _app,
                                       4,
                                       ((_app->GetOutputSampleRate() / 2) / 4) / 2,
                                       AF);
}

void MainWindow::SetupNavigationMenu() {
    _menubar->add("Navigation/Frequency/Up 1KHz", ("+^" + std::to_string(FL_Up)).c_str(), HandleFrequencyNavigationCallback, (void*) this);
    _menubar->add("Navigation/Frequency/Up 100Hz", ("^" + std::to_string(FL_Up)).c_str(), HandleFrequencyNavigationCallback, (void*) this);
    _menubar->add("Navigation/Frequency/Up 10Hz", ("+" + std::to_string(FL_Up)).c_str(), HandleFrequencyNavigationCallback, (void*) this);
    _menubar->add("Navigation/Frequency/Up",  + std::to_string(FL_Up).c_str(), HandleFrequencyNavigationCallback, (void*) this);
    _menubar->add("Navigation/Frequency/Down", std::to_string(FL_Down).c_str(), HandleFrequencyNavigationCallback, (void*) this);
    _menubar->add("Navigation/Frequency/Down 10Hz", ("+" + std::to_string(FL_Down)).c_str(), HandleFrequencyNavigationCallback, (void*) this);
    _menubar->add("Navigation/Frequency/Down 100Hz", ("^" + std::to_string(FL_Down)).c_str(), HandleFrequencyNavigationCallback, (void*) this);
    _menubar->add("Navigation/Frequency/Down 1KHz", ("+^" + std::to_string(FL_Down)).c_str(), HandleFrequencyNavigationCallback, (void*) this, FL_MENU_DIVIDER);
    _menubar->add("Navigation/Frequency/Frequency", "^f", HandleMenuCtrlF, (void*) this);
    _menubar->add("Navigation/Frequency/Offset", "^o", HandleMenuCtrlO, (void*) this);
}

/***********************************************
  Callback handlers
***********************************************/

/**
 * Handle clicks on menubuttons
 * @param name Name of the button clicked
 */
void MainWindow::HandleMenuButton(char* name) {

    if( strcmp(name, "File/Quit") == 0 ) {
        Exit();
    }
    else if( strncmp(name, "Configuration/Inputs/", 21) == 0 ) {
        HandleMenuButtonConfigurationInputs(name, &name[21]);
    }
    else if( strncmp(name, "Receiver/Dump RF", 16) == 0 || strncmp(name, "Receiver/Stop dumping RF", 24) == 0 ) {
        HandleMenuButtonReceiverDumpRf();
    }
    else if( strncmp(name, "Receiver/Dump AF", 16) == 0 || strncmp(name, "Receiver/Stop dumping AF", 24) == 0 ) {
        HandleMenuButtonReceiverDumpAf();
    }
    else if( strncmp(name, "Receiver/Start", 14) == 0 || strncmp(name, "Receiver/Stop", 13) == 0 ) {
        HandleMenuButtonReceiverStartStop();
    }
    else if( strncmp(name, "Receiver/Restart", 16) == 0 ) {
        HandleMenuButtonReceiverRestart();
    }
    else if( strncmp(name, "Receiver/Input/", 15) == 0 ) {
        HandleMenuButtonReceiverInput(name, &name[15]);
    }
    else if( strncmp(name, "Receiver/Output/", 16) == 0 ) {
        HandleMenuButtonReceiverOutput(name, &name[16]);
    }
    else if( strncmp(name, "Receiver/Mode/", 14) == 0 ) {
        HandleMenuButtonReceiverMode(name, &name[14]);
    }
    else if( strncmp(name, "Receiver/Settings/IF filter width/", 34) == 0 ) {
        HandleMenuButtonReceiverIfFilterWidth(name, &name[34]);
    }
    else if( strncmp(name, "Receiver/Settings/Preamp", 24) == 0 ) {
        HandleMenuButtonReceiverPreamp(name, &name[24]);
    }
    else if( strncmp(name, "Receiver/Settings/", 18) == 0 ) {
        HandleMenuButtonReceiverOptions(name, &name[18]);
    }
    else if( strcmp(name, "Help/Help") == 0 ) {
        // Todo: Show proper splash
        std::cout << GetTitle() << std::endl;
    }
    else {
        HError("Unknown menubutton '%s' clicked", name);
    }
}

void MainWindow::HandleMenuButtonReceiverStartStop() {

    if( _app->IsRunning() ) {
        Halt();
    } else {
        Run();
    }
}

void MainWindow::HandleMenuButtonReceiverRestart() {
    _app->ChangeReceiver();
    Run();
}

/**
 * Handle click on any of the 'Receiver/Input/xx' menubuttons
 * @param name Full name of menubutton clicked
 * @param value Name of the button (part after 'Receiver/Input/' aka The input name
 */
void MainWindow::HandleMenuButtonReceiverInput(char* name, char* value) {

    Halt();

    // Handle 'Edit' button
    if( strncmp(value, "Edit ", 5) == 0 ) {
        EditReceiverInput(_app->GetConfigSection().c_str());
        return;
    }

    // Change configuration
    std::string previousConfig = _app->GetConfigSection();
    try {
        _app->SetConfigSection(value);
    } catch( ... ) {
        fl_alert("There was an error when selecting the new input.\nCheck that the settings are valid\n");
        _app->SetConfigSection(previousConfig);
    }

    // Update the menu
    SetupReceiverInputMenu();
    SetupConfigurationMenu();
    SetupChannels();
    SetupChannels();

    _volumeSlider->value(_app->GetVolume());
    _gainSlider->value(MapToGainSliderValue(_app->GetRfGain()));
    _frequencyInput->value(std::to_string(_app->GetFrequency()).c_str());
    _volumeSlider->redraw();
    _gainSlider->redraw();
    SetGainSliderLabel();
    SetVolumeSliderLabel();
    _rfInputWaterfall->ReConfigure(_app->GetInputSourceDataType() != REAL_INPUT_SOURCE_DATA_TYPE, 1);

    Run();
}

/**
 * Handle click on any of the 'Receiver/Output/xx' menubuttons
 * @param name Full name of menubutton clicked
 * @param value Name of the button (part after 'Receiver/Output/' aka The output device
 */
void MainWindow::HandleMenuButtonReceiverOutput(char* name, char* value) {

    // Halt receiver
    Halt();

    // No output
    if( strncmp(value, "Silence", 7) == 0 ) {
        _app->SetOutputAudioDevice(-1);
    }

    // Write output audio to a file
    else if( strncmp(value, "File", 4) == 0 ) {

        auto tstamp = std::time(nullptr);
        auto tm = *std::localtime(&tstamp);
        std::ostringstream oss;
        oss << std::put_time(&tm, "AUDIO_%d-%m-%Y_%H-%M-%S.wav");

        _app->SetOutputFilename(oss.str());
    }

    // Physical and virtual audio cards
    else if( strncmp(value, "Card ", 5) == 0 ) {
        int card = atoi(&value[5]);
        _app->SetOutputAudioDevice(card);
    }

    // Unknown menu item
    else {
        HError("Unknown card or device '%s'", value);
    }

    // Update the menu
    SetupReceiverOutputMenu();

    // Restart
    _app->ChangeReceiver();
    Run();
}

/**
 * Handle click on any of the 'Receiver/Mode/xx' menubuttons
 * @param name Full name of menubutton clicked
 * @param value Name of the button (part after 'Receiver/Mode/' aka The mode name
 */
void MainWindow::HandleMenuButtonReceiverMode(char* name, char* value) {

    // Halt running receiver
    Halt();

    char fallback[16];
    switch(_app->GetReceiver()) {
        case AURORAL:
            strcpy(fallback, "AURORAL");
            break;
        case AM:
            strcpy(fallback, "AM");
            break;
        case CW:
            strcpy(fallback, "CW");
            break;
        case SSB:
            strcpy(fallback, "SSB");
            break;
        default:
            HError("No receiver mode set");
            throw new BoomaReceiverException("No receiver mode set");
    }

    char requested[16];
    strcpy(requested, value);

    bool created = false;
    do {
        try {
            // Change receiver mode
            if (strcmp(requested, "AURORAL") == 0) {
                created = _app->ChangeReceiver(ReceiverModeType::AURORAL);
            } else if (strcmp(requested, "AM") == 0) {
                created = _app->ChangeReceiver(ReceiverModeType::AM);
            } else if (strcmp(requested, "CW") == 0) {
                created = _app->ChangeReceiver(ReceiverModeType::CW);
            } else if (strcmp(requested, "SSB") == 0) {
                created = _app->ChangeReceiver(ReceiverModeType::SSB);
            } else {
                HError("Unknown receiver mode '%s'", requested);
                throw new BoomaReceiverException("Unknown receiver mode");
            }

            if( !created ) {
                HError("Receiver creation failed, falling back to %s", fallback);
                strcpy(requested, fallback);
            }
        }
        catch ( ... ) {
            HError("Caught unknown exception while changing receiver");
            throw new BoomaReceiverException("Caught unknown exception while changing receiver");
        }
    } while(!created);

    // Add options for selected receiver
    SetupSettingsMenu();

    // Update frequency controls - it may change when selecting a new receiver
    _frequencyInput->value(std::to_string(_app->GetFrequency()).c_str());
    _frequencyInput->redraw();

    // Update the receiver menu
    SetupReceiverMenu();

    // Restart receiver
    Run();
}


/**
 * Handle click on any of the 'Receiver/Settings/Preamp' menubuttons for the receiver preamp level
 * @param name Full name of menubutton clicked
 * @param value Value of the selected level
 */
void MainWindow::HandleMenuButtonReceiverPreamp(char* name, char* value) {

    // Set the selected radio button
    const_cast<Fl_Menu_Item*>(_menubar->find_item(const_cast<const char*>(name)))->setonly();

    if( strcmp(value, "/-12 dB") == 0 ) {
        _app->SetPreampLevel(-1);
    } else if( strcmp(value, "/+12 dB") == 0 ) {
        _app->SetPreampLevel(1);
    } else if( strcmp(value, "/+24 dB") == 0 ) {
        _app->SetPreampLevel(2);
    } else {
        _app->SetPreampLevel(0);
    }
}

/**
 * Handle click on any of the 'Receiver/Settings/xx' menubuttons for the receiver options
 * @param name Full name of menubutton clicked
 * @param value Value of the selected option
 */
void MainWindow::HandleMenuButtonReceiverOptions(char* name, char* value) {

    // Set the selected radio button
    const_cast<Fl_Menu_Item*>(_menubar->find_item(const_cast<const char*>(name)))->setonly();

    // Split into option and value
    char* sep = strchr(value, '/');
    char option[50] = {0};
    char newValue[50] = {0};
    strncpy(option, value, sep - value);
    strncpy(newValue, sep + 1, strlen(sep + 1));

    // Find the new option value
    for( std::vector<Option>::iterator  it = _app->GetOptions()->begin(); it != _app->GetOptions()->end(); it++ ) {
        if( strcmp((*it).Name.c_str(), option) == 0 ) {
            for( std::vector<OptionValue>::iterator vit = (*it).Values.begin(); vit != (*it).Values.end(); vit++ ) {
                if( strcmp((*vit).Name.c_str(), newValue) == 0 ) {
                    _app->SetOption(option, (*vit).Name);
                    return;
                }
            }
        }
    }
}

void MainWindow::HandleMenuButtonConfigurationInputs(char* name, char* value) {
    if( strncmp(value, "Add", 3) == 0 ) {
        AddReceiverInput();
    } else if( strncmp(value, "Delete ", 7) == 0 ) {
        DeleteReceiverInput(&value[7]);
    }
}

void MainWindow::HandleMenuButtonReceiverIfFilterWidth(char* name, char* value) {

    // Set the selected radio button
    const_cast<Fl_Menu_Item*>(_menubar->find_item(const_cast<const char*>(name)))->setonly();

    // Set width (width comes in Hz)
    if( strcmp(value, "off") == 0 ) {
        _app->SetInputFilterWidth(0);
    } else {
        _app->SetInputFilterWidth(atoi(value));
    }
}

void MainWindow::HandleFrequencyInputButtons(Fl_Widget *w) {
    if( w == _frequencyInputSet ) {
        try
        {
            _app->SetFrequency(std::stol(_frequencyInput->value()));
        }
        catch(...)
        { }
        _frequencyInput->value(std::to_string(_app->GetFrequency()).c_str());
    }
    if( w == _frequencyInputDown1Khz ) {
        _app->ChangeFrequency(-1000);
    }
    if( w == _frequencyInputDown100 ) {
        _app->ChangeFrequency(-100);
    }
    if( w == _frequencyInputUp100 ) {
        _app->ChangeFrequency(100);
    }
    if( w == _frequencyInputUp1Khz ) {
        _app->ChangeFrequency(1000);
    }
    _frequencyInput->value(std::to_string(_app->GetFrequency()).c_str());
}

void MainWindow::HandleFrequencyInput(Fl_Widget *w) {
    try
    {
        _app->SetFrequency(std::stol(_frequencyInput->value()));
    }
    catch(...)
    { }
    _frequencyInput->value(std::to_string(_app->GetFrequency()).c_str());
    _frequencyInput->redraw();
    Fl::focus(_win);
}

void MainWindow::HandleFrequencyOffset(Fl_Widget *w) {
    Halt();

    try
    {
        _app->SetOffset(atoi(_frequencyOffset->value()));
    }
    catch(...)
    { }

    _app->ChangeReceiver();

    _frequencyOffset->redraw();
    Fl::focus(_win);

    Run();
}

void MainWindow::HandleChannelSelector(Fl_Widget *w) {
    _app->SetFrequency(_app->GetChannels().at(_channelSelector->value() + 1)->Frequency);
    _frequencyInput->value(std::to_string(_app->GetFrequency()).c_str());
}

void MainWindow::HandleGainSlider() {
    _app->SetRfGain(MapFromGainSliderValue(_gainSlider->value()));
    SetGainSliderLabel();
}

void MainWindow::HandleGainEnabled() {
    _app->SetRfGainEnabled(!_app->GetRfGainEnabled());
    _gainEnabled->value(_app->GetRfGainEnabled() ? 1 : 0);
    if( _app->GetRfGainEnabled() ) {
        _gainSlider->set_active();
        _gainSlider->redraw();
    } else {
        _gainSlider->clear_active();
        _gainSlider->redraw();
    }
    SetGainSliderLabel();
}

void MainWindow::HandleVolumeSlider() {
    _app->SetVolume(_volumeSlider->value());
    SetVolumeSliderLabel();
}

void MainWindow::HandleRfWaterfall() {
    _app->SetFrequency(_rfInputWaterfall->GetSelectedFrequency());
    _frequencyInput->value(std::to_string(_app->GetFrequency()).c_str());
}

void MainWindow::HandleFrequencyNavigation(char* name) {
    char value[80];
    char *p = strrchr(name, '/');
    if( p != nullptr ) {
        strcpy(value, p + 1);
        if( strcmp(value, "Up 1KHz") == 0 ) {
            _app->ChangeFrequency(1000);
        } else if( strcmp(value, "Up 100Hz") == 0 ) {
            _app->ChangeFrequency(100);
        } else if( strcmp(value, "Up 10Hz") == 0 ) {
            _app->ChangeFrequency(10);
        } else if( strcmp(value, "Up") == 0 ) {
            _app->ChangeFrequency(1);
        } else if( strcmp(value, "Down") == 0 ) {
            _app->ChangeFrequency(-1);
        } else if( strcmp(value, "Down 10Hz") == 0 ) {
            _app->ChangeFrequency(-10);
        } else if( strcmp(value, "Down 100Hz") == 0 ) {
            _app->ChangeFrequency(-100);
        } else if( strcmp(value, "Down 1KHz") == 0 ) {
            _app->ChangeFrequency(-1000);
        }
        _frequencyInput->value(std::to_string(_app->GetFrequency()).c_str());
    }
}

void MainWindow::HandleCtrlF() {
    if( Fl::focus() == _frequencyInput ) {
        Fl::focus(_win);
    } else {
        Fl::focus(_frequencyInput);
        _frequencyInput->redraw();
    }
}

void MainWindow::HandleCtrlO() {
    if( Fl::focus() == _frequencyOffset ) {
        Fl::focus(_win);
    } else {
        Fl::focus(_frequencyOffset);
        _frequencyOffset->redraw();
    }
}

void MainWindow::HandleEscape() {
    _frequencyInput->value(std::to_string(_app->GetFrequency()).c_str());
    _frequencyInput->redraw();
    _frequencyOffset->value(std::to_string(_app->GetOffset()).c_str());
    _frequencyOffset->redraw();
    Fl::awake();

    Fl::focus(_win);
}

void MainWindow::EditReceiverInput(const char* name) {
    Halt();

    try {
        InputDialog *dlg = new InputDialog(_app, InputDialog::Mode::EDIT);
        if (dlg->Show()) {
            _app->ChangeReceiver();
            _app->SyncConfiguration();
        }
        delete (dlg);
    } catch( ... ) {
        fl_alert("There was an error when using the edited input.\nCheck that the settings are valid\n");
    }

    _volumeSlider->value(MapToGainSliderValue(_app->GetVolume()));
    _gainSlider->value(_app->GetRfGain());
    _frequencyInput->value(std::to_string(_app->GetFrequency()).c_str());
    _volumeSlider->redraw();
    _gainSlider->redraw();

    SetupReceiverInputMenu();
    SetupConfigurationMenu();

    Run();
}

void MainWindow::AddReceiverInput() {
    int outputDevice = _app->GetOutputDevice();
    int volume = _app->GetVolume();

    Halt();

    InputDialog* dlg = new InputDialog(_app, InputDialog::Mode::ADD);
    if( dlg->Show() ) {
        _app->ChangeReceiver();
        _app->SyncConfiguration();
    }
    delete(dlg);

    _app->SetOutputAudioDevice(outputDevice);
    _app->SetVolume(volume);
    _volumeSlider->value(MapToGainSliderValue(volume));
    _gainSlider->value(_app->GetRfGain());
    _frequencyInput->value(std::to_string(_app->GetFrequency()).c_str());
    _volumeSlider->redraw();
    _gainSlider->redraw();

    SetupChannels();
    SetupReceiverOutputMenu();
    SetupReceiverInputMenu();
    SetupConfigurationMenu();

    Run();
}

void MainWindow::DeleteReceiverInput(const char* name) {

    // We cannot delete the section if no other sections exists
    std::vector<std::string> configurations = _app->GetConfigSections();
    if( configurations.size() == 1 ) {
        HError("Can not delete the last configuration section");
        return;
    }

    // We cannot delete the active configuration
    if( _app->GetConfigSection() == name ) {
        HError("Not allowed to delete the active configuration");
        return;
    }

    // Delete the configuration
    _app->DeleteConfigSection(name);

    // Refresh the Configuration/Inputs menu
    SetupConfigurationMenu();
    SetupReceiverInputMenu();
}

void MainWindow::RemoveMenuSubItems(const char *name) {
    const char* head = name;
    const Fl_Menu_Item* submenu = _menubar->menu();

    // Find the (sub)menu containing the items to be deleted
    while( *head != '\0' ) {

        // Get name of section
        char section[50] = {0};
        const char *sep = strchr(head, '/');
        if (sep == NULL) {
            break;
        }
        strncpy(section, head, sep - head);

        // Try to locate this section
        bool found = false;
        for ( int i = 0; i < submenu->size(); i++ ) {
            const Fl_Menu_Item& item = submenu->first()[i];
            if (item.label() != NULL && strcmp(item.label(), section) == 0 ) {
                submenu = &item;
                found = true;
                break;
            }
        }
        if( found == false ) {
            return;
        }

        // Next section/item
        head = sep + 1;
    }

    // Set search pattern
    char pattern[50];
    strcpy(pattern, head);
    if( pattern[strlen(pattern) - 1] == '*' ) {
        pattern[strlen(pattern) - 1] = '\0';
    }

    // Remove any matching items in the selected submenu
    int i = 1;
    while( i < submenu->size() ) {
        const Fl_Menu_Item& item = submenu->first()[i];

        if (item.label() != NULL && strncmp(item.label(), pattern, strlen(pattern)) == 0) {
            for ( int i = 0; i < _menubar->size(); i++ ) {
                const Fl_Menu_Item &mitem = _menubar->menu()[i];
                if( &mitem == &item ) {
                    _menubar->remove(i);
                    break;
                }
            }
        } else if( item.label() == NULL ) {
            break;
        }
        else {
            i++;
        }
    }
}

void MainWindow::RenameMenuItem(const char *name, const char* newname) {
    const Fl_Menu_Item* item = _menubar->find_item(name);
    if( item == nullptr ){
        return;
    }
    const_cast<Fl_Menu_Item*>(item)->label(newname);
}

int MainWindow::MapFromGainSliderValue(long value) {
    if( value < -10 ) {
        return value + 10;
    } else if( value > 10 ) {
        return value - 10;
    } else {
        return 0;
    }
}

long MainWindow::MapToGainSliderValue(int value) {
    if( value < 0 ) {
        return value -10;
    } else if( value > 0 ) {
        return value + 10;
    } else {
        return 0;
    }
}

void MainWindow::SetGainSliderLabel() {
    if (_app->GetRfGainEnabled()) {
        strcpy(_gainLabel,("  RF Gain (" + (_app->GetRfGain() == 0 ? "auto" : std::to_string(_app->GetRfGain())) + ")  ").c_str());
    } else {
        strcpy(_gainLabel, "  RF Gain (off)  ");
    }
    _gainSlider->label(_gainLabel);
}

void MainWindow::SetVolumeSliderLabel() {
    strcpy(_volumeLabel, ("  Volume (" + std::to_string(_app->GetVolume()) + ")  ").c_str());
    _volumeSlider->label(_volumeLabel);
}

inline void MainWindow::UpdateSignalLevelDisplay() {
    const char* levels[12] = { "      S0      ", "      S1      ", "      S2      ", "      S3      ", "      S4      ", "      S5      ", "      S6      ", "      S7      ", "      S8      ", "      S9      ", " S9 +10dB ", " S9 +20dB " };

    static int initialize = 0;
    if( initialize < 25 ) {
        _signalLevelSlider->label(levels[0]);
        _signalLevelSlider->scrollvalue(0, 1,0, 12);
        _signalLevelSlider->color(FL_GRAY, FL_GRAY);
        initialize++;
        return;
    }

    int level = _app->GetSignalLevel();
    static int previousLevel = 0;
    if( level != previousLevel ) {
        switch( _app->GetSignalLevel() ) {
            case 0:
                _signalLevelSlider->label(levels[0]);
                _signalLevelSlider->scrollvalue(0, 1,0, 12);
                _signalLevelSlider->color(FL_GRAY, FL_GRAY);
                break;
            case 1:
                _signalLevelSlider->label(levels[1]);
                _signalLevelSlider->scrollvalue(1, 1,0, 12);
                _signalLevelSlider->color(FL_GRAY, FL_YELLOW);
                break;
            case 2:
                _signalLevelSlider->label(levels[2]);
                _signalLevelSlider->scrollvalue(2, 1,0, 12);
                _signalLevelSlider->color(FL_GRAY, FL_YELLOW);
                break;
            case 3:
                _signalLevelSlider->label(levels[3]);
                _signalLevelSlider->scrollvalue(3, 1,0, 12);
                _signalLevelSlider->color(FL_GRAY, FL_GREEN);
                break;
            case 4:
                _signalLevelSlider->label(levels[4]);
                _signalLevelSlider->scrollvalue(4, 1,0, 12);
                _signalLevelSlider->color(FL_GRAY, FL_GREEN);
                break;
            case 5:
                _signalLevelSlider->label(levels[5]);
                _signalLevelSlider->scrollvalue(5, 1,0, 12);
                _signalLevelSlider->color(FL_GRAY, FL_GREEN);
                break;
            case 6:
                _signalLevelSlider->label(levels[6]);
                _signalLevelSlider->scrollvalue(6, 1,0, 12);
                _signalLevelSlider->color(FL_GRAY, FL_GREEN);
                break;
            case 7:
                _signalLevelSlider->label(levels[7]);
                _signalLevelSlider->scrollvalue(7, 1,0, 12);
                _signalLevelSlider->color(FL_GRAY, FL_GREEN);
                break;
            case 8:
                _signalLevelSlider->label(levels[8]);
                _signalLevelSlider->scrollvalue(8, 1,0, 12);
                _signalLevelSlider->color(FL_GRAY, FL_GREEN);
                break;
            case 9:
                _signalLevelSlider->label(levels[9]);
                _signalLevelSlider->scrollvalue(9, 1,0, 12);
                _signalLevelSlider->color(FL_GRAY, FL_GREEN);
                break;
            case 10:
                _signalLevelSlider->label(levels[10]);
                _signalLevelSlider->scrollvalue(10, 1,0, 12);
                _signalLevelSlider->color(FL_GRAY, FL_DARK_YELLOW);
                break;
            case 11:
                _signalLevelSlider->label(levels[11]);
                _signalLevelSlider->scrollvalue(11, 1, 0, 12);
                _signalLevelSlider->color(FL_GRAY, FL_RED);
                break;
            default:
                if( level > 11) {
                    _signalLevelSlider->label(levels[11]);
                    _signalLevelSlider->scrollvalue(11, 1, 0, 12);
                    _signalLevelSlider->color(FL_GRAY, FL_RED);
                } else {
                    _signalLevelSlider->label(levels[0]);
                    _signalLevelSlider->scrollvalue(0, 1,0, 12);
                    _signalLevelSlider->color(FL_GRAY, FL_YELLOW);
                }
                break;
        }
        previousLevel = level;
        Fl::awake();
    }
}

inline void MainWindow::UpdateRfSpectrumDisplay() {
    _app->GetRfSpectrum(_rfInputWaterfall->GetFftBuffer());
    _rfInputWaterfall->Refresh();
}

inline void MainWindow::UpdateAfSpectrumDisplay() {
    _app->GetAudioSpectrum(_afOutputWaterfall->GetFftBuffer());
    _afOutputWaterfall->Refresh();
}

void MainWindow::Run() {
    try {
        _app->Run();
        _threadsPaused = false;
    } catch( ... ) {
        fl_alert("Could not start the receiver. Check the settings");
    }
    UpdateState();
}

void MainWindow::Halt() {
    _app->Halt();
    _threadsPaused = true;
    UpdateState();
}

void MainWindow::HandleMenuButtonReceiverDumpRf() {
    _app->ToggleDumpRf();
    UpdateState();
}

void MainWindow::HandleMenuButtonReceiverDumpAf() {
    _app->ToggleDumpAudio();
    UpdateState();
}

void MainWindow::UpdateState() {

    // Running/Stopped
    if( _app->IsRunning() ) {
        RenameMenuItem("Receiver/Start", "Stop");
    } else {
        RenameMenuItem("Receiver/Stop", "Start");
    }

    // Dump RF
    if( _app->GetDumpRf() ) {
        RenameMenuItem("Receiver/Dump RF", "Stop dumping RF");
    } else {
        RenameMenuItem("Receiver/Stop dumping RF", "Dump RF");
    }

    // Dump AF
    if( _app->GetDumpAudio() ) {
        RenameMenuItem("Receiver/Dump AF", "Stop dumping AF");
    } else {
        RenameMenuItem("Receiver/Stop dumping AF", "Dump AF");
    }

    // Statusbar
    UpdateStatusbar();

    // Offset control
    if( _frequencyOffset != nullptr ) {
        if (_app->GetInputSourceType() == RTLSDR || _app->GetOriginalInputSourceType() == RTLSDR) {
            _frequencyOffset->set_active();
            _frequencyOffset->color(FL_WHITE);
        } else {
            _frequencyOffset->clear_active();
            _frequencyOffset->color(FL_GRAY);
        }
        _frequencyOffset->redraw();
    }

    if( _frequencyInput != nullptr ) {
        if (_app->GetInputSourceType() == PCM_FILE && _app->GetInputSourceDataType() != REAL_INPUT_SOURCE_DATA_TYPE) {
            _frequencyInput->clear_active();
            _frequencyInput->color(FL_GRAY);
        } else {
            _frequencyInput->set_active();
            _frequencyInput->color(FL_WHITE);
        }
        _frequencyInput->redraw();
    }
}

void MainWindow::UpdateStatusbar() {

    if( _statusbar == nullptr ) {
        return;
    }
    _statusbar->redraw();

    _statusbarConfig->value(_app->GetConfigSection().c_str());

    switch(_app->GetReceiver()) {
        case ReceiverModeType::AM:
            _statusbarMode->value("AM");
            break;
        case ReceiverModeType::AURORAL:
            _statusbarMode->value("AURORAL");
            break;
        case ReceiverModeType::CW:
            _statusbarMode->value("CW");
            break;
        case ReceiverModeType::SSB:
            _statusbarMode->value("SSB");
            break;
        default:
            _statusbarMode->value("(none)");
            break;
    }

    if (_app->GetInputSourceDataType() == REAL_INPUT_SOURCE_DATA_TYPE) {
        _statusbarHardwareFreq->value(std::to_string(_app->GetFrequency()).c_str());
    } else {
        _statusbarHardwareFreq->value(std::to_string(_app->GetFrequency() + _app->GetShift() + _app->GetFrequencyAdjust()).c_str());
    }

    if( _app->IsRunning() ) {
        _statusbarRunningState->textcolor(FL_GREEN);
        _statusbarRunningState->value("Running");
    } else {
        _statusbarRunningState->textcolor(FL_RED);
        _statusbarRunningState->value("Stopped");
    }

    _statusbarRecordingRf->value( _app->GetDumpRf() ? "Recording RF" : "");
    _statusbarRecordingAf->value( _app->GetDumpAudio() ? "Recording AF" : "");
}
