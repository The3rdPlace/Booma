#include <stdio.h>
#include <iostream>
#include <signal.h>

#include "main.h"
#include "boomaapplication.h"

#include <FL/Fl.H>
#include <FL/fl_ask.H>

#include "mainwindow.h"

MainWindow* mainWindow = nullptr;
BoomaApplication* app = nullptr;

void SetupSignalHandling()
{
    struct sigaction actionIntTerm;
    actionIntTerm.sa_handler = [](int) { std::cout << "int/term\n"; mainWindow->Exit(); };
    actionIntTerm.sa_flags = 0;
    sigemptyset (&actionIntTerm.sa_mask);
    sigaction (SIGINT, &actionIntTerm, NULL);
    sigaction (SIGTERM, &actionIntTerm, NULL);

    struct sigaction actionAbrt;
    actionAbrt.sa_handler = [](int) {
        char message[] = "Received SIGABRT, this indicates that there is something\n" \
                         "really bad with either your input or output!\n" \
                         "\n" \
                         "Check that your audio or rtl2832 device is connected\n" \
                         "\n" \
                         "If that is not the cause, then try starting booma again with\n" \
                         "the '-i SILENCE -o -1' options to run with a minimal number of\n" \
                         "devices, then make the necessary changes to the active input.\n" \
                         "\n" \
                         "Otherwise, restart booma with the '-z' switch to reset your\n" \
                         "configuration and start with a minimal default configuration\n";

        fl_alert("%s", message);
        std::cout << message;

        exit(1);
    };
    actionAbrt.sa_flags = 0;
    sigemptyset (&actionAbrt.sa_mask);
    sigaction (SIGABRT, &actionAbrt, NULL);
}

int main(int argc, char** argv) {

    // Setup signal handling so that we can exit cleanly on ctrl+c
    SetupSignalHandling();

    // Create the mainwindow with the booma application
    try {

        std::stringstream ss;
        ss << BOOMAGUI_MAJORVERSION << "." << BOOMAGUI_MINORVERSION << "." << BOOMAGUI_BUILDNO;
        try {
            app = new BoomaApplication("Booma-Gui", ss.str(), argc, argv);
        } catch( ... ) {
            std::cout << "App failed to initialize. Could be a bad configuration.\n\n";
            std::cout << "If you have forgotten to plug in the device used by the\n";
            std::cout << "default (current) receiver, plug it in and retry, otherwise\n";
            std::cout << "you can reset the configuration with the command 'booma-gui -z\n";exit(1);
        }

        // Todo: For strange reasons, the input filter fails to initialize properly
        //       when having a default input filter size of 0 (=off)
        if( !app->IsFaulty() ) {
            app->SetInputFilterWidth(app->GetInputFilterWidth());
        }

        // Create the mainwindow
        mainWindow = new MainWindow(app);

        // Run the main event loop
        Fl::run();

        // Destroy the mainwindow and booma application
        delete(mainWindow);
        delete(app);

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
