#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#include "main.h"
#include "booma.h"
#include <thread>
#include <chrono>

int main(int argc, char** argv) {

	// Initialize Booma
	std::stringstream ss;
    ss << "version " << BOOMACONSOLE_MAJORVERSION << "." << BOOMACONSOLE_MINORVERSION << "." << BOOMACONSOLE_BUILDNO;
	BoomaApplication app("Booma-Console", ss.str(), argc, argv, true);

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

        // Repeat last command ?
        if( cmd == '\n' ) {
            cmd = lastCmd;
            opt = lastOpt;
        }
        else
        {
            // Does the command requires an option ?
            if( cmd == 'f' || cmd == 'g' || cmd == 'v' || cmd == 'r' ) {
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
                if( !app.ChangeFrequency( frequency ) ) {
                    std::cout << "Unsupported frequency" << std::endl;
                }
            }
            else if( opt.at(0) == '-' ) {
                frequency = atoi(opt.substr(1, std::string::npos).c_str());
                if( !app.ChangeFrequency( -1 * frequency ) ){
                    std::cout << "Unsupported frequency" << std::endl;
                }
            }
            else
            {
                frequency = atoi(opt.c_str());
                if( !app.SetFrequency(frequency) ) {
                    std::cout << "Unsupported frequency" << std::endl;
                }
            }
        }

        // Increase/Decrease volume
        else if( cmd == 'v' ) {
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
        else if( cmd == 'p' ) {
            app.ToggleDumpRf();
        }
        else if( cmd == 'a' ) {
            app.ToggleDumpAudio();
        }

        // Increase/decrease rf gain
        else if( cmd == 'g' ) {
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

        // Change receiver type
        else if( cmd == 'r' ) {
            if( opt == "CW" ) {
                app.ChangeReceiver(ReceiverModeType::CW);
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                app.Run();
            }
            else
            {
                std::cout << "Unknown receiver type" << std::endl;
            }
        }
        else if( cmd == '?' ) {
            std::cout << "f ###  *  g [+|-]###  *  v [+|-]###  *  a  *  p  *  r CW  *  q" << std::endl;
        }

        // Unknown command
        else if( cmd != 'q') {
            std::cout << "Unknown command !" << std::endl;
            while( std::cin.get() != '\n' ) {}
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
