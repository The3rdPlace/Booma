#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#include "booma.h"

int main(int argc, char** argv) {

	// Initialize Booma
	BoomaApplication app(argc, argv, true);

    // Run initial receiver (if any configured)
    app.Run();

    // Read commands
    std::string cmd;
    do {
        std::cout << "Booma# ";
        std::cin >> cmd;

        std::cout << "read " << cmd << std::endl;

        // Increase/Decrease frequency
        if( cmd == "u" ) {
            app.ChangeFrequency(100);
        }
        if( cmd == "d" ) {
            app.ChangeFrequency(-100);
        }

        // Increase/Decrease volume
        if( cmd == "+" ) {
            app.ChangeVolume(10);
        }
        if( cmd == "-" ) {
            app.ChangeVolume(-10);
        }

        // Dump pcm/audio
        if( cmd == "p" ) {
            app.ToggleDumpPcm();
        }
        if( cmd == "a" ) {
            app.ToggleDumpAudio();
        }
    }
    while( cmd != "q" && cmd != "quit" );

    // Stop a running receiver (if any)
    app.Halt();

    // Leave peacefully
	return 0;
}
