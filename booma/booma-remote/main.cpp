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

	// Leave peacefully
	return 0;
}
