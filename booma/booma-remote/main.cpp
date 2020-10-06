#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <signal.h>

#include "main.h"
#include "booma.h"
#include "boomaapplication.h"

BoomaApplication* app;

void SetupSignalHandling()
{
    struct sigaction action;
    action.sa_handler = [](int) { app->Halt(false); };
    action.sa_flags = 0;
    sigemptyset (&action.sa_mask);
    sigaction (SIGINT, &action, NULL);
    sigaction (SIGTERM, &action, NULL);
}

int main(int argc, char** argv) 
{
	// Initialize Booma
	std::stringstream ss;
	ss << "version " << BOOMAREMOTE_MAJORVERSION << "." << BOOMAREMOTE_MINORVERSION << "." << BOOMAREMOTE_BUILDNO;
	try {
        app = new BoomaApplication("Booma-Remote", ss.str(), argc, argv);

        // Setup a signal handler so that we can exit cleanly
        SetupSignalHandling();

        // Run initial receiver (if any configured)
        app->Run();
        std::cout << "booma-remote " << ss.str() << " running. Press ctrl+c to quit" << std::endl;
        app->Wait();
        std::cout << std::endl << "booma-remote finished" << std::endl;

        // Cleanup
        delete app;
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

	// Leave peacefully
	return 0;
}
