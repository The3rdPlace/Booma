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
    char cmd;
    char lastCmd;
    std::string opt;
    std::string lastOpt;
    do {
        std::cout << "Booma [ f=" << app.GetFrequency() << ", v=" << app.GetVolume() << ", rf.g=" << app.GetRfGain() << " ]# ";
        cmd = (char) std::cin.get();

        // Remove leading whitespace
        if( cmd == ' ' ) {
            cmd = (char) std::cin.get();
        }

        // Repeat last command ?
        if( cmd == '\n' ) {
            cmd = lastCmd;
            opt = lastOpt;
        }
        else
        {
            // Does the command requires options ?
            if( cmd == 'f' || cmd == 'g' || cmd == 'v' ) {
                std::cin >> opt;
            }
            else
            {
                opt = "";
            }
            std::cin.get();
        }

        // Set/Increase/Decrease frequency
        if( cmd == 'f' ) {
            int frequency;

            if( opt.at(0) == '+' ) {
                frequency = atoi(opt.substr(1, std::string::npos).c_str());
                app.ChangeFrequency( frequency );
            }
            else if( opt.at(0) == '-' ) {
                frequency = atoi(opt.substr(1, std::string::npos).c_str());
                app.ChangeFrequency( -1 * frequency );
            }
            else
            {
                frequency = atoi(opt.c_str());
                app.SetFrequency(frequency);
            }
        }

        // Increase/Decrease volume
        if( cmd == 'v' ) {
            int volume;

            if( opt.at(0) == '+' ) {
                volume = atoi(opt.substr(1, std::string::npos).c_str());
                app.ChangeVolume( volume );
            }
            else if( opt.at(0) == '-' ) {
                volume = atoi(opt.substr(1, std::string::npos).c_str());
                app.ChangeVolume( -1 * volume );
            }
            else
            {
                volume = atoi(opt.c_str());
                app.SetVolume( volume );
            }
        }

        // Dump pcm/audio
        if( cmd == 'p' ) {
            app.ToggleDumpPcm();
        }
        if( cmd == 'a' ) {
            app.ToggleDumpAudio();
        }

        // Increase/decrease rf gain
        if( cmd == 'g' ) {
            int gain;

            if( opt.at(0) == '+' ) {
                gain = atoi(opt.substr(1, std::string::npos).c_str());
                app.ChangeRfGain( gain );
            }
            else if( opt.at(0) == '-' ) {
                gain = atoi(opt.substr(1, std::string::npos).c_str());
                app.ChangeRfGain( -1 * gain );
            }
            else
            {
                gain = atoi(opt.c_str());
                app.SetRfGain(gain);
            }

        }

        // Save last command and options
        lastCmd = cmd;
        lastOpt = opt;
    }
    while( cmd != 'q' );

    // Stop a running receiver (if any)
    app.Halt();

    // Leave peacefully
	return 0;
}
