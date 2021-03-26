#include <stdio.h>
#include <iostream>
#include <signal.h>

#include "main.h"
#include "boomaapplication.h"

#include "mainwindow.h"

BoomaApplication* boomaApp;
Glib::RefPtr<Gtk::Application> app;

void SetupSignalHandling()
{
    struct sigaction action;
    action.sa_handler = [](int) { boomaApp->Halt(false); app->quit(); };
    action.sa_flags = 0;
    sigemptyset (&action.sa_mask);
    sigaction (SIGINT, &action, NULL);
    sigaction (SIGTERM, &action, NULL);
}

int main(int argc, char** argv) {

    // Initialize GTK and Create the  mainwindow
    app = Gtk::Application::create("org.hardttoolkit.booma.booma-gui");
    SetupSignalHandling();

    // Initialize the Booma application and start the main loop
    int rc;
    std::stringstream ss;
    ss << "version " << BOOMAGUI_MAJORVERSION << "." << BOOMAGUI_MINORVERSION << "." << BOOMAGUI_BUILDNO;
    try {
        boomaApp = new BoomaApplication("Booma-Gui", ss.str(), argc, argv);

        // Run initial receiver (if any configured)
        boomaApp->Run();

        // Run the main event loop
        MainWindow window;
        rc = app->run(window, argc, argv);

        // Halt and delete the booma application
        boomaApp->Halt();
        delete boomaApp;
    }
    catch( BoomaReceiverException* receiverException ) {
        HError("Caught BoomaReceiverException: %s", receiverException->what());
        std::cout << "Fatal receiver error (" << receiverException->What() << ")" << std::endl;
        return 1;
    }
    catch( BoomaException *boomaException ) {
        HError("Caught BoomaException: %s", boomaException->what());
        std::cout << "Caught unexpected internal exception (" << boomaException->What() << ")" << std::endl;
        return 1;
    }
    catch( ... ) {
        HError("Caught unknown exception");
        std::cout << "Caught unknown exception" << std::endl;
        return 1;
    }

    return rc;
}
