#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#include "booma.h"

int main(int argc, char** argv) {

	// Initialize Booma
	BoomaApplication app(argc, argv, false);

    // Run initial receiver (if any configured)
    app.Run();

    // Read commands
    std::string cmd;
    do {
        std::cout << "Booma [" << app.GetFrequency() << "]# ";
        std::cin >> cmd;

        // Set/Increase/Decrease frequency
        if( cmd == "f" ) {
            std::cin >> cmd;
            int frequency = atoi(cmd.c_str());
            app.SetFrequency(frequency);
        }
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

        // Increase/decrease rf gain
        if( cmd == "g" ) {
            std::cin >> cmd;
            int gain = atoi(cmd.c_str());
            app.SetRfGain(gain);
        }
        if( cmd == "r") {
            app.ChangeRfGain(5);
        }
        if( cmd == "e") {
            app.ChangeRfGain(-5);
        }
    }
    while( cmd != "q" && cmd != "quit" );

    // Stop a running receiver (if any)
    app.Halt();

    // Leave peacefully
	return 0;
}
