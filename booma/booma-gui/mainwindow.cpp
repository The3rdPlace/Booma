#include <hardt.h>
#include "main.h"
#include "booma.h"
#include "mainwindow.h"

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
    delete(_app);
    // Todo: remove own widgets
}

/**
 * Stop receiver and exit the mainwindow - effectively closing the application
 */
void MainWindow::Exit() {

    // Halt the receiver
    _app->Halt();

    // Cleanup
    delete(_app);

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
    _menubar->add("File/Quit", "^q", HandleMenuButtonCallback, (void*) this);

    // Receiver
    SetupReceiverInputMenu();
    SetupReceiverOutputMenu();
    SetupReceiverFilters();
    SetupReceiverRfGainMenu();
    _menubar->add("Receiver/Mode/AURORAL", 0, HandleMenuButtonCallback, (void*) this,
                  FL_MENU_RADIO | (_app->GetReceiver() == ReceiverModeType::AURORAL ? FL_MENU_VALUE : 0));
    _menubar->add("Receiver/Mode/AM", 0, HandleMenuButtonCallback, (void*) this,
                  FL_MENU_RADIO | (_app->GetReceiver() == ReceiverModeType::AM ? FL_MENU_VALUE : 0));
    _menubar->add("Receiver/Mode/CW", 0, HandleMenuButtonCallback, (void*) this,
                  FL_MENU_RADIO | (_app->GetReceiver() == ReceiverModeType::CW ? FL_MENU_VALUE : 0));
    _menubar->add("Receiver/Mode/SSB", 0, HandleMenuButtonCallback, (void*) this,
                  FL_MENU_RADIO | (_app->GetReceiver() == ReceiverModeType::SSB ? FL_MENU_VALUE : 0));

    // Options
    SetupOptionsMenu();

    // Help
    _menubar->add("Help/Help", 0, HandleMenuButtonCallback, (void*) this);
}

void MainWindow::SetupReceiverInputMenu() {
    std::vector<std::string> configSections = _app->GetConfigSections();
    for( int i = 0; i < configSections.size(); i++ ) {
        _menubar->add(("Receiver/Input/" + configSections.at(i)).c_str(), 0, HandleMenuButtonCallback, (void*) this,
                FL_MENU_RADIO |
                (_app->GetConfigSection() == configSections.at(i) ? FL_MENU_VALUE : 0) |
                (i == configSections.size() - 1 ? FL_MENU_DIVIDER : 0));
    }
    _menubar->add("Receiver/Input/Add new input", 0, HandleMenuButtonCallback, (void*) this);
}


void MainWindow::SetupReceiverOutputMenu() {
    // Todo: Hook this up
    _menubar->add("Receiver/Output/USB soundcard 1", 0, HandleMenuButtonCallback, (void*) this,
                  FL_MENU_RADIO | FL_MENU_VALUE);
}

void MainWindow::SetupReceiverFilters() {

    // Todo: Hook this up

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

    // Define some usefull defaults for the output filter cutoff frequency
    _menubar->add("Receiver/Filters/Output filter cutoff/1 KHz", 0, HandleMenuButtonCallback, (void*) this,
                  FL_MENU_RADIO | (_app->GetOutputFilterWidth() == 1000 ? FL_MENU_VALUE : 0));
    _menubar->add("Receiver/Filters/Output filter cutoff/3 KHz", 0, HandleMenuButtonCallback, (void*) this,
                  FL_MENU_RADIO | (_app->GetOutputFilterWidth() == 3000 ? FL_MENU_VALUE : 0));
    _menubar->add("Receiver/Filters/Output filter cutoff/5 KHz", 0, HandleMenuButtonCallback, (void*) this,
                  FL_MENU_RADIO | (_app->GetOutputFilterWidth() == 5000 ? FL_MENU_VALUE : 0));

    // Allow an arbitrary value for the output filter cutoff frequency from manual configuration
    if( _app->GetOutputFilterWidth() != 1000 && _app->GetOutputFilterWidth() != 3000 &&
        _app->GetOutputFilterWidth() != 5000 ) {
        _menubar->add(("Receiver/Filters/Output filter cutoff/" + std::to_string(_app->GetOutputFilterWidth()) + " Hz").c_str(), 0, HandleMenuButtonCallback, (void*) this,
                      FL_MENU_RADIO | FL_MENU_VALUE);
    }
}

void MainWindow::SetupReceiverRfGainMenu() {

    // Todo: Hook this  up
    
    // Define some usefull defaults for rf gain
    _menubar->add("Receiver/RF gain/auto", 0, HandleMenuButtonCallback, (void*) this,
                  FL_MENU_RADIO | (_app->GetRfGain() == 0 ? FL_MENU_VALUE : 0));
    _menubar->add("Receiver/RF gain/-9 DB", 0, HandleMenuButtonCallback, (void*) this,
                  FL_MENU_RADIO | (_app->GetRfGain() == -8 ? FL_MENU_VALUE : 0));
    _menubar->add("Receiver/RF gain/-6 DB", 0, HandleMenuButtonCallback, (void*) this,
                  FL_MENU_RADIO | (_app->GetRfGain() == -4 ? FL_MENU_VALUE : 0));
    _menubar->add("Receiver/RF gain/-3 DB", 0, HandleMenuButtonCallback, (void*) this,
                  FL_MENU_RADIO | (_app->GetRfGain() == -2 ? FL_MENU_VALUE : 0));
    _menubar->add("Receiver/RF gain/0 DB", 0, HandleMenuButtonCallback, (void*) this,
                  FL_MENU_RADIO | (_app->GetRfGain() == 1 ? FL_MENU_VALUE : 0));
    _menubar->add("Receiver/RF gain/3 DB", 0, HandleMenuButtonCallback, (void*) this,
                  FL_MENU_RADIO | (_app->GetRfGain() == 2 ? FL_MENU_VALUE : 0));
    _menubar->add("Receiver/RF gain/6 DB", 0, HandleMenuButtonCallback, (void*) this,
                  FL_MENU_RADIO | (_app->GetRfGain() == 4 ? FL_MENU_VALUE : 0));
    _menubar->add("Receiver/RF gain/9 DB", 0, HandleMenuButtonCallback, (void*) this,
                  FL_MENU_RADIO | (_app->GetRfGain() == 8 ? FL_MENU_VALUE : 0));
    _menubar->add("Receiver/RF gain/12 DB", 0, HandleMenuButtonCallback, (void*) this,
                  FL_MENU_RADIO | (_app->GetRfGain() == 16 ? FL_MENU_VALUE : 0));
    _menubar->add("Receiver/RF gain/15 DB", 0, HandleMenuButtonCallback, (void*) this,
                  FL_MENU_RADIO | (_app->GetRfGain() == 32 ? FL_MENU_VALUE : 0));
    _menubar->add("Receiver/RF gain/18 DB", 0, HandleMenuButtonCallback, (void*) this,
                  FL_MENU_RADIO | (_app->GetRfGain() == 64 ? FL_MENU_VALUE : 0));


    // Allow an arbitrary value for the output filter cutoff frequency from manual configuration
    if( _app->GetRfGain() != 0 && _app->GetRfGain() != -8 && _app->GetRfGain() != -4 &&
            _app->GetRfGain() != -2 && _app->GetRfGain() != 0 && _app->GetRfGain() != 2 &&
            _app->GetRfGain() != 4 && _app->GetRfGain() != 8 && _app->GetRfGain() != 16 &&
            _app->GetRfGain() != 32 && _app->GetRfGain() != 64 ) {
        _menubar->add(("Receiver/RF gain/(" + std::to_string(_app->GetRfGain()) + ")").c_str(), 0, HandleMenuButtonCallback, (void*) this,
                      FL_MENU_RADIO | FL_MENU_VALUE);
    }
}

void MainWindow::SetupOptionsMenu() {

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
}

/**
 * Setup display widgets in the main window
 */
void MainWindow::SetupDisplays() {

    // RF Input waterfall
    _rfInputWaterfall = new Waterfall(10, _menubar->h() + 10, 300, 200, "RF input");
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
    else if( strncmp(name, "Receiver/Mode/", 14) == 0 ) {
        HandleMenuButtonReceiverMode(name, &name[14]);
    }
    else if( strncmp(name, "Receiver/Options/", 17) == 0 ) {
        HandleMenuButtonReceiverOptions(name, &name[17]);
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
 * Handle click on any of the 'Receiver/Mode/xx' menubuttons
 * @param name Full name of menubutton clicked
 * @param value Name of the button (part after 'Receiver/Mode/' aka The mode name
 */
void MainWindow::HandleMenuButtonReceiverMode(char* name, char* value) {

    // Remove all options
    for ( int i = 0; i < _menubar->size(); i++ ) {
        const Fl_Menu_Item &item = _menubar->menu()[i];
        if( item.label() != NULL && strcmp(item.label(), "Options") == 0 ) {
            _menubar->remove(i);
            break;
        }
    }

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
            std::cout << "IS " << (*it).Name << std::endl;
            for( std::vector<OptionValue>::iterator vit = (*it).Values.begin(); vit != (*it).Values.end(); vit++ ) {
                if( strcmp((*vit).Name.c_str(), newValue) == 0 ) {
                    _app->SetOption(option, (*vit).Name);
                    return;
                }
            }
        }
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
}


void MainWindow::HandleChannelSelector(Fl_Widget *w) {
    _app->SetFrequency(_app->GetChannels().at(_channelSelector->value() + 1)->Frequency);
    _frequencyInput->value(std::to_string(_app->GetFrequency()).c_str());
}
