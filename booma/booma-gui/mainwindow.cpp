#include <hardt.h>
#include "main.h"
#include "booma.h"
#include "mainwindow.h"

/**
 * Static callback for menu handling
 * @param w The menubar widget
 * @param data Pointer to the mainwindow
 */
void HandleMenuButtonCallback(Fl_Widget* w, void* data) {
    MainWindow* mainWindow = (MainWindow*) data;

    char name[80];
    ((Fl_Menu_Button*) w)->item_pathname(name, sizeof(name)-1);

    mainWindow->HandleMenuButton(name);
}

/**
 * Construct the mainwindow
 * @param app Initialized BoomaApplication
 */
MainWindow::MainWindow(BoomaApplication* app):
    _app(app) {

    // Create the window
    _win = new Fl_Window(720, 486, GetTitle());
    SetupMenus();
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

/**
 * Create the menubar
 */
void MainWindow::SetupMenus() {
    _menubar = new Fl_Menu_Bar(0, 0, _win->w(), 25);
    _menubar->add("File/Quit", "^q", HandleMenuButtonCallback, (void*) this);
    _menubar->add("Help/Help", 0, HandleMenuButtonCallback, (void*) this);
}

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
