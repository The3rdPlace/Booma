#include <hardt.h>
#include <iomanip>
#include "main.h"
#include "booma.h"
#include "mainwindow.h"
#include "inputdialog.h"

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
 * Static callback for gain slider events
 * @param w
 */
void HandleVolumeSliderCallback(Fl_Widget* w) {
    MainWindow::Instance()->HandleVolumeSlider();
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
    SetupMenus();
    SetupControls();
    SetupDisplays();
    _win->end();
    _win->show();

    // Start the receiver
    _app->Run();
}

/**
 * Destruct the mainwindow
 */
MainWindow::~MainWindow() {

    // Halt the receiver
    _app->Halt();

    // Cleanup
    // Todo: remove own widgets
}

/**
 * Stop receiver and exit the mainwindow - effectively closing the application
 */
void MainWindow::Exit() {

    // Halt the receiver
    _app->Halt();

    // Close window
    exit(0);
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

    // Receiver
    SetupReceiverInputMenu();
    SetupReceiverOutputMenu();
    SetupReceiverInputFilterMenu();
    SetupReceiverModeMenu();

    // Options
    SetupOptionsMenu();

    // Help
    _menubar->add("Help/Help", 0, HandleMenuButtonCallback, (void*) this);
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
        _menubar->add(("Receiver/Output/Physical card "  + std::to_string((*it).first) + ": " + (*it).second).c_str(), 0, HandleMenuButtonCallback, (void *) this,
                      FL_MENU_RADIO | (_app->GetOutputDevice() == (*it).first ? FL_MENU_VALUE : 0));
    }

    std::map<int, std::string> virtualCards = _app->GetAudioDevices(false, true, false, true);
    for( std::map<int, std::string>::iterator it = virtualCards.begin(); it != virtualCards.end(); it++) {
        _menubar->add(("Receiver/Output/Virtual card "  + std::to_string((*it).first) + ": " + (*it).second).c_str(), 0, HandleMenuButtonCallback, (void *) this,
                      FL_MENU_RADIO | (_app->GetOutputDevice() == (*it).first ? FL_MENU_VALUE : 0));
    }

    _menubar->add("Receiver/Output/Silence", 0, HandleMenuButtonCallback, (void *) this,
                  FL_MENU_RADIO | (_app->GetOutputDevice() == -1 && _app->GetOutputFilename() == "" ? FL_MENU_VALUE : 0));

    _menubar->add("Receiver/Output/File", 0, HandleMenuButtonCallback, (void *) this,
                  FL_MENU_RADIO | (_app->GetOutputFilename() != "" ? FL_MENU_VALUE : 0));
}

void MainWindow::SetupReceiverInputFilterMenu() {

    // Define some usefull defaults for the IF filter
    _menubar->add("Receiver/Filters/IF filter width/500 Hz", 0, HandleMenuButtonCallback, (void*) this,
                  FL_MENU_RADIO | (_app->GetInputFilterWidth() == 500 ? FL_MENU_VALUE : 0));
    _menubar->add("Receiver/Filters/IF filter width/1 KHz", 0, HandleMenuButtonCallback, (void*) this,
                  FL_MENU_RADIO | (_app->GetInputFilterWidth() == 1000 ? FL_MENU_VALUE : 0));
    _menubar->add("Receiver/Filters/IF filter width/3 KHz", 0, HandleMenuButtonCallback, (void*) this,
                  FL_MENU_RADIO | (_app->GetInputFilterWidth() == 3000 ? FL_MENU_VALUE : 0));
    _menubar->add("Receiver/Filters/IF filter width/5 KHz", 0, HandleMenuButtonCallback, (void*) this,
                  FL_MENU_RADIO | (_app->GetInputFilterWidth() == 5000 ? FL_MENU_VALUE : 0));
    _menubar->add("Receiver/Filters/IF filter width/10 KHz", 0, HandleMenuButtonCallback, (void*) this,
                  FL_MENU_RADIO | (_app->GetInputFilterWidth() == 10000 ? FL_MENU_VALUE : 0));

    // Allow an arbitrary value for the IF filter from manual configuration
    if( _app->GetInputFilterWidth() != 500 && _app->GetInputFilterWidth() != 1000 &&
            _app->GetInputFilterWidth() != 3000 && _app->GetInputFilterWidth() != 5000 &&
            _app->GetInputFilterWidth() != 10000 ) {
        _menubar->add(("Receiver/Filters/IF filter width/" + std::to_string(_app->GetInputFilterWidth()) + " Hz").c_str(), 0, HandleMenuButtonCallback, (void*) this,
                      FL_MENU_RADIO | FL_MENU_VALUE);
    }
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

void MainWindow::SetupOptionsMenu() {

    RemoveMenuSubItems("Receiver/Options/*");

    // If no options is available, show a disabled 'Options' submenu
    if( _app->GetOptions()->begin() == _app->GetOptions()->end() ) {
        _menubar->add("Receiver/Options", 0, HandleMenuButtonCallback, (void*) this,
                      FL_MENU_INACTIVE);
        return;
    }

    // Otherwise show available options
    for( std::vector<Option>::iterator it = _app->GetOptions()->begin(); it != _app->GetOptions()->end(); it++ ) {
        std::vector<OptionValue> values = (*it).Values;
        for( std::vector<OptionValue>::iterator vit = values.begin(); vit != values.end(); vit++ ) {
            _menubar->add( ("Receiver/Options/" + (*it).Name + "/" + (*vit).Name).c_str(), 0, HandleMenuButtonCallback, (void*) this,
                           FL_MENU_RADIO | ( (*vit).Value == (*it).CurrentValue ? FL_MENU_VALUE : 0) );
        }
    }
}

/**
 * Setup controls in the main window
 */
void MainWindow::SetupControls() {

    // Frequency input
    _frequencyInput = new Fl_Input(90, _win->h() - 40, 150, 30, "Frequency ");
    _frequencyInput->value(std::to_string(_app->GetFrequency()).c_str());
    _frequencyInputSet = new Fl_Button(250, _win->h() - 40, 50, 30, "set");
    _frequencyInputDown1Khz = new Fl_Button(320, _win->h() - 40, 30, 30, "<<");
    _frequencyInputDown100 = new Fl_Button(350, _win->h() - 40, 30, 30, "<");
    _frequencyInputUp100 = new Fl_Button(385, _win->h() - 40, 30, 30, ">");
    _frequencyInputUp1Khz = new Fl_Button(415, _win->h() - 40, 30, 30, ">>");
    _frequencyInput->callback(HandleFrequencyInputCallback);
    _frequencyInput->when(FL_WHEN_ENTER_KEY);
    _frequencyInputSet->callback(HandleFrequencyInputButtonsCallback);
    _frequencyInputUp1Khz->callback(HandleFrequencyInputButtonsCallback);
    _frequencyInputUp100->callback(HandleFrequencyInputButtonsCallback);
    _frequencyInputDown100->callback(HandleFrequencyInputButtonsCallback);
    _frequencyInputDown1Khz->callback(HandleFrequencyInputButtonsCallback);

    // Channel selector
    _channelSelector = new Fl_Choice(465, _win->h() - 40, 245, 30);
    std::map<int, Channel*> channels = _app->GetChannels();
    for( std::map<int, Channel*>::iterator it = channels.begin(); it != channels.end(); it++ ) {
        _channelSelector->add(((*it).second->Name).c_str());
    }
    _channelSelector->callback(HandleChannelSelectorCallback);

    // Gain and volume sliders
    strcpy(_gainLabel, ("RF Gain (" + (_app->GetRfGain() == 0 ? "auto" : std::to_string(_app->GetRfGain())) + ")").c_str());
    _gainSlider = new Fl_Slider(FL_HOR_NICE_SLIDER, _win->w() - 160, _menubar->h() + 10, 150, 30, _gainLabel);
    _gainSlider->bounds(-20, 110);
    _gainSlider->scrollvalue(MapToGainSliderValue(_app->GetRfGain()), 1, -20, 131);
    _gainSlider->callback(HandleGainSliderCallback);
    strcpy(_volumeLabel, ("Volume (" + std::to_string(_app->GetVolume()) + ")").c_str());
    _volumeSlider = new Fl_Slider(FL_HOR_NICE_SLIDER, _win->w() - 160, _menubar->h() + 80, 150, 30, _volumeLabel);
    _volumeSlider->bounds(0, 30);
    _volumeSlider->scrollvalue(_app->GetVolume(), 1, 0, 31);
    _volumeSlider->callback(HandleVolumeSliderCallback);
}

/**
 * Setup display widgets in the main window
 */
void MainWindow::SetupDisplays() {

    // RF Input waterfall
    _rfInputWaterfall = new Waterfall(10, _menubar->h() + 10, _win->w() - 180, 200, "RF input");
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
    else if( strncmp(name, "Receiver/Input/", 15) == 0 ) {
        HandleMenuButtonReceiverInput(name, &name[15]);
    }
    else if( strncmp(name, "Receiver/Output/", 16) == 0 ) {
        HandleMenuButtonReceiverOutput(name, &name[16]);
    }
    else if( strncmp(name, "Receiver/Mode/", 14) == 0 ) {
        HandleMenuButtonReceiverMode(name, &name[14]);
    }
    else if( strncmp(name, "Receiver/Options/", 17) == 0 ) {
        HandleMenuButtonReceiverOptions(name, &name[17]);
    }
    else if( strncmp(name, "Receiver/Filters/IF filter width/", 33) == 0 ) {
        HandleMenuButtonReceiverIfFilterWidth(name, &name[33]);
    }
    else if( strcmp(name, "Help/Help") == 0 ) {
        // Todo: Show proper splash
        std::cout << GetTitle() << std::endl;
    }
    else {
        HError("Unknown menubutton '%s' clicked", name);
    }
}

/**
 * Handle click on any of the 'Receiver/Input/xx' menubuttons
 * @param name Full name of menubutton clicked
 * @param value Name of the button (part after 'Receiver/Input/' aka The input name
 */
void MainWindow::HandleMenuButtonReceiverInput(char* name, char* value) {

    // Handle 'Edit' button
    if( strncmp(value, "Edit ", 5) == 0 ) {
        EditReceiverInput(_app->GetConfigSection().c_str());
        return;
    }

    // Change configuration
    _app->SetConfigSection(value);

    // Update the menu
    SetupReceiverInputMenu();
    _app->Run();
}

/**
 * Handle click on any of the 'Receiver/Output/xx' menubuttons
 * @param name Full name of menubutton clicked
 * @param value Name of the button (part after 'Receiver/Output/' aka The output device
 */
void MainWindow::HandleMenuButtonReceiverOutput(char* name, char* value) {

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
    else if( strncmp(value, "Physical card ", 14) == 0 ) {
        int card = atoi(&value[14]);
        _app->SetOutputAudioDevice(card);
    } else if( strncmp(value, "Virtual card ", 13) == 0 ) {
        int card = atoi(&value[13]);
        _app->SetOutputAudioDevice(card);
    }

    // Unknown menu item
    else {
        HError("Unknown card or device '%s'", value);
    }

    // Update the menu
    SetupReceiverOutputMenu();
    _app->Run();
}

/**
 * Handle click on any of the 'Receiver/Mode/xx' menubuttons
 * @param name Full name of menubutton clicked
 * @param value Name of the button (part after 'Receiver/Mode/' aka The mode name
 */
void MainWindow::HandleMenuButtonReceiverMode(char* name, char* value) {

    // Change receiver mode
    if( strcmp(value, "AURORAL") == 0 ) {
        _app->ChangeReceiver(ReceiverModeType::AURORAL);
    } else if( strcmp(value, "AM") == 0 ) {
        _app->ChangeReceiver(ReceiverModeType::AM);
    } else if( strcmp(value, "CW") == 0 ) {
        _app->ChangeReceiver(ReceiverModeType::CW);
    } else if( strcmp(value, "SSB") == 0 ) {
        _app->ChangeReceiver(ReceiverModeType::SSB);
    } else {
        HError("Unknown receiver mode '%s'", value);
    }

    // Add options for selected receiver
    SetupOptionsMenu();
}

/**
 * Handle click on any of the 'Receiver/Options/xx' menubuttons
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

    // Set width (width comes in KHz)
    _app->SetInputFilterWidth(atoi(value) * 1000);
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
}


void MainWindow::HandleChannelSelector(Fl_Widget *w) {
    _app->SetFrequency(_app->GetChannels().at(_channelSelector->value() + 1)->Frequency);
    _frequencyInput->value(std::to_string(_app->GetFrequency()).c_str());
}

void MainWindow::HandleGainSlider() {
    strcpy(_gainLabel, ("   RF Gain (" + (MapFromGainSliderValue(_gainSlider->value()) == 0 ? "auto" : std::to_string(MapFromGainSliderValue(_gainSlider->value()))) + ")   ").c_str());
    _gainSlider->label(_gainLabel);
    _app->SetRfGain(MapFromGainSliderValue(_gainSlider->value()));
}

void MainWindow::HandleVolumeSlider() {
    strcpy(_volumeLabel, ("   Volume (" + std::to_string((int) _volumeSlider->value()) + ")   ").c_str());
    _volumeSlider->label(_volumeLabel);
    _app->SetVolume(_volumeSlider->value());
}

void MainWindow::EditReceiverInput(const char* name) {
    InputDialog* dlg = new InputDialog(_app, InputDialog::Mode::EDIT);
    if( dlg->Show() ) {
        _app->ChangeReceiver();
        _app->Run();
    }
    delete(dlg);

    SetupReceiverInputMenu();
    SetupConfigurationMenu();
}

void MainWindow::AddReceiverInput() {
    InputDialog* dlg = new InputDialog(_app, InputDialog::Mode::ADD);
    if( dlg->Show() ) {
        _app->ChangeReceiver();
        _app->Run();
    }
    delete(dlg);

    SetupReceiverInputMenu();
    SetupConfigurationMenu();
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

        // Get name of config section
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
            if (item.label() != NULL && strcmp(item.label(), section) == 0) {
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
