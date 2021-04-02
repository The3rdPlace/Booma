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
    _menubar->add("File/Quit", "^q", HandleMenuButtonCallback, (void*) this);
    _menubar->add("Help/Help", 0, HandleMenuButtonCallback, (void*) this);
}

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
    if( strcmp(name, "Help/Help") == 0 ) {
        // Todo: Show proper splash
        std::cout << GetTitle() << std::endl;
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
