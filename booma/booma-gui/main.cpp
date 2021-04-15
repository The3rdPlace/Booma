#include <stdio.h>
#include <iostream>
#include <signal.h>

#include "main.h"
#include "boomaapplication.h"

#include <FL/Fl.H>

#include "mainwindow.h"

MainWindow* mainWindow = nullptr;
BoomaApplication* app = nullptr;

void SetupSignalHandling()
{
    struct sigaction action;
    action.sa_handler = [](int) { mainWindow->Exit(); };
    action.sa_flags = 0;
    sigemptyset (&action.sa_mask);
    sigaction (SIGINT, &action, NULL);
    sigaction (SIGTERM, &action, NULL);
}

int main(int argc, char** argv) {

    // Setup signal handling so that we can exit cleanly on ctrl+c
    SetupSignalHandling();

    // Create the mainwindow with the booma application
    try {

        std::stringstream ss;
        ss << BOOMAGUI_MAJORVERSION << "." << BOOMAGUI_MINORVERSION << "." << BOOMAGUI_BUILDNO;
        app = new BoomaApplication("Booma-Gui", ss.str(), argc, argv);

        // Todo: For strange reasons, the input filter fails to initialize properly
        //       when having a default input filter size of 0 (=off)
        app->SetInputFilterWidth(app->GetInputFilterWidth());

        // Create the mainwindow
        mainWindow = new MainWindow(app);

        // Run the main event loop
        Fl::run();

        // Destroy the mainwindow and booma application
        std::cout << "delete mainwindow" << std::endl;
        delete(mainWindow);
        std::cout << "delete app" << std::endl;
        delete(app);
std::cout << "stop" << std::endl;
        // Return for normal exit
        return 0;
    }
    catch( BoomaReceiverException* receiverException ) {
        HError("Caught BoomaReceiverException: %s", receiverException->what());
        std::cout << "Fatal receiver error (" << receiverException->What() << ")" << std::endl;
        if( mainWindow != nullptr ) {
            delete(mainWindow);
        }
        if( app != nullptr ) {
            delete(app);
        }
        return 1;
    }
    catch( BoomaException *boomaException ) {
        HError("Caught BoomaException: %s", boomaException->what());
        std::cout << "Caught unexpected internal exception (" << boomaException->What() << ")" << std::endl;
        if( mainWindow != nullptr ) {
            delete(mainWindow);
        }
        if( app != nullptr ) {
            delete(app);
        }
        return 2;
    }
    catch( ... ) {
        HError("Caught unknown exception");
        std::cout << "Caught unknown exception" << std::endl;
        if( mainWindow != nullptr ) {
            delete(mainWindow);
        }
        if( app != nullptr ) {
            delete(app);
        }
        return 3;
    }
}
