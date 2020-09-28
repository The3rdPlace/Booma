#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <termios.h>
#include <unistd.h>

#include "main.h"
#include "booma.h"
#include "info.h"
#include <thread>
#include <chrono>
#include <future>

void signalMeasurementWorker(std::future<void> future, BoomaApplication* app, bool* shouldMark)
{
    while( future.wait_for(std::chrono::milliseconds(1000)) == std::future_status::timeout ) {
        if( *shouldMark ) {
            std::cout << "\r                     \r"  << app->GetSignalSum() << "  M" << std::endl;
            *shouldMark = false;
        } else {
            std::cout << "\r                     \r"  << app->GetSignalSum();
        }
        std::cout.flush();
    }
}

std::string TranslateReceiverModeType(ReceiverModeType type) {
    switch(type) {
        case ReceiverModeType::AURORAL: return "Auroral";
        case ReceiverModeType::CW: return "CW";
        case ReceiverModeType::AM: return "AM";
        case ReceiverModeType::SSB: return "SSB";
        default: return "UNKNOWN_RECEIVER";
    }
}

std::string ComposeInfoPrompt(BoomaApplication* app) {
    return "Booma [ " +
    TranslateReceiverModeType(app->GetReceiver()) +
    " f=" + std::to_string(app->GetFrequency()) + ( app->GetOptionInfoString() != "" ? "[" + app->GetOptionInfoString() + "]" : "") +
    " v=" + std::to_string(app->GetVolume()) +
    " gain=" + std::to_string(app->GetRfGain()) +
    (app->GetDumpRf() ? " " : "") +
    (app->GetDumpRf() ? (app->GetEnableBuffers() ? "RF(b)" : "RF") : "") +
    (app->GetDumpAudio() ? " " : "") +
    (app->GetDumpAudio() ? (app->GetEnableBuffers() ? "AUDIO(b)" : "AUDIO") : "") +
    " ]# ";
}

int main(int argc, char** argv) {

	// Initialize Booma
	std::stringstream ss;
    ss << "version " << BOOMACONSOLE_MAJORVERSION << "." << BOOMACONSOLE_MINORVERSION << "." << BOOMACONSOLE_BUILDNO;
	BoomaApplication app("Booma-Console", ss.str(), argc, argv);

    // Wait for scheduled start time or stop now if we have passed a scheduled stop time
    if( app.GetSchedule().Before() ) {
        HLog("Scheduled start is pending. Waiting %ld seconds", app.GetSchedule().Duration());
        std::cout << "Scheduled start is pending. Waiting..." << std::endl;
        app.GetSchedule().Wait();
        std::cout << "Scheduled start time has arrived. Starting..." << std::endl;
    }
    if( app.GetSchedule().After() ) {
        HLog("After scheduled stop, halting application");
        std::cout << "Scheduled stop has been reached. Will exit now" << std::endl;
        return 0;
    }

    // Run initial receiver (if any configured)
    if( app.GetEnableProbes() ) {
        HLog("Probe run is starting");
        std::cout << "Probe run is starting" << std::endl;
    }
    app.Run();
    if( app.GetEnableProbes() ) {
        HLog("Probe run has been completed. Will exit now");
        std::cout << "Probe run has been completed. Will exit now" << std::endl;
        return 0;
    }

	// Create an Info object to report various informations from the receiver
	Info info(&app);

    // If we have a schedule, non-interactive mode is selected
    if( app.GetSchedule().Active() ) {
        std::cout << "Running in non-interactive mode due to schedule" << std::endl;
        while( !app.GetSchedule().After() ) {
            sleep(1);
        }
        std::cout << "Scheduled stop time has been reached. Stopping..." << std::endl;
        app.Halt();
        return 0;
    }

    // Read commands
    char cmd;
    char lastCmd;
    std::string opt;
    std::string lastOpt;
    std::cout << "Running in interactive mode. Press '?' or 'h' to get help." << std::endl;
    do {
        std::cout << ComposeInfoPrompt(&app);
        cmd = (char) std::cin.get();

        // Repeat last command ?
        if( cmd == '\n' ) {
            cmd = lastCmd;
            opt = lastOpt;
        }
        else
        {
            // Does the command requires an option ?
            if( cmd == 'f' || cmd == 'g' || cmd == 'v' || cmd == 'r' || cmd == 'o' || cmd == 'b' || cmd == 'c' || cmd == 'd') {
                std::cin >> opt;
            }
            else
            {
                opt = "";
            }
            while( std::cin.get() != '\n' ) {}
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

        // Restart with current receiver
        else if( cmd == 's' ) {
            app.ChangeReceiver();
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            app.Run();
        }

        // Get help
        else if( cmd == '?' || cmd == 'h' ) {
            std::cout << "-------------------------------------------------------------------------------------" << std::endl;
            std::cout << "Change frequency:               f <frequency>  or  f +<amount>  or  -<amount>" << std::endl;
            std::cout << "Change RF gain:                 g <gain>       or  g +<amount>  or  -<amount>" << std::endl;
            std::cout << "Change volume:                  v <volume>     or  v +<amount>  or  -<amount>" << std::endl;
            std::cout << "Change receiver type:           r <CW|CW2|AURORAL> or  s (use current receiver and input)" << std::endl;
            std::cout << "Set receiver option:            o <NAME=VALUE>" << std::endl;
            std::cout << "Toggle audio recording on/off:  a" << std::endl;
            std::cout << "Toggle rf recording on/off:     p" << std::endl;
            std::cout << "Enter measurement mode:         m" << std::endl;
            std::cout << "Set bookmark:                   b <name>" << std::endl;
            std::cout << "Get bookmark:                   c <name>" << std::endl;
            std::cout << "Delete bookmark                 d" << std::endl;            
            std::cout << "List bookmarks:                 x" << std::endl;
            std::cout << std::endl;
            std::cout << "Press enter on a blank line to repeat the last command" << std::endl;
            std::cout << std::endl;
            std::cout << "Get help (this text):           ?  or  h" << std::endl;
            std::cout << "Get reception status:           i" << std::endl;
            std::cout << "List receiver options:          l" << std::endl;
            std::cout << "Quit:                           q" << std::endl;
            std::cout << "-------------------------------------------------------------------------------------" << std::endl;
        }

        // Get info
        else if( cmd == 'i' ) {
            info.GetInfo();
        }

        // Show a running meter indicating a relative signal power - for comparing antennas and their placement.
        // The measurement is not really comparable outside your own location and equipment, but it can be used
        // to gauge where the antenna is best placed on your property.
        else if( cmd == 'm' ) {

            // Disable keyboard echo. This allows us to properly 'mark' selected measurements
            #ifdef WIN32
            #warning Assumed to be correct for windows, but needs checking
            HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE); 
            DWORD mode = 0;
            GetConsoleMode(hStdin, &mode);
            SetConsoleMode(hStdin, mode & (~ENABLE_ECHO_INPUT));
            #else
            termios termAttr;
            tcgetattr(STDIN_FILENO, &termAttr);
            termAttr.c_lflag &= ~ECHO;
            tcsetattr(STDIN_FILENO, TCSANOW, &termAttr);
            #endif

            std::promise<void> p;
            std::future<void> f = p.get_future();
            bool shouldMark = false;
            std::thread t(signalMeasurementWorker, std::move(f), &app, &shouldMark);
            
            std::cout << std::endl << "Relative signal measurement (press 'm'+enter to mark, 'q'+enter to exit):" << std::endl << "0";
            char subCmd;
            do {
                subCmd = std::cin.get();
                if( subCmd == 'm' ) {
                    shouldMark = true;
                }
                std::cin.get();
            } while( subCmd != 'q' );
            
            p.set_value();
            t.join();
            std::cout << "\r                     \r" << std::endl;

            // Reenable keyboard echo. This allows us to properly 'mark' selected measurements
            #ifdef WIN32
            #warning Assumed to be correct for windows, but needs checking
            GetConsoleMode(hStdin, &mode);
            SetConsoleMode(hStdin, mode | ENABLE_ECHO_INPUT);
            #else
            tcgetattr(STDIN_FILENO, &termAttr);
            termAttr.c_lflag |= ECHO;
            tcsetattr(STDIN_FILENO, TCSANOW, &termAttr);
            #endif
        }

        // Show receiver options
        else if( cmd == 'l' ) {
            std::vector<Option>* options = app.GetOptions();
            for( std::vector<Option>::iterator it = options->begin(); it != options->end(); it++ ) {
                int current = app.GetOption((*it).Name);
                std::cout << (*it).Name << ":" << std::endl;
                for( std::vector<OptionValue>::iterator valIt = (*it).Values.begin(); valIt != (*it).Values.end(); valIt++ ) {
                    if( (*valIt).Value == current ) {
                        std::cout << "* " << (*valIt).Name << " (" << (*valIt).Description << ") = " << (*valIt).Value << std::endl;
                    } else {
                        std::cout << "  " << (*valIt).Name << " (" << (*valIt).Description << ") = " << (*valIt).Value << std::endl;
                    }
                }
            }
        }

        // Set receiver options
        else if( cmd == 'o' ) {
            if( opt.size() == 0 ) {
                std::cout << "Missing option name and value" << std::endl;
                continue;
            }
            int pos = opt.find("=");
            if( pos < 0 ) {
                std::cout << "Option name and value must be given as 'NAME=VALUE'" << std::endl;
                continue;
            }
            if( pos == 0 ) {
                std::cout << "Option 'NAME' can not be empty" << std::endl;
                continue;
            }
            if( pos >= opt.size() - 1 ) {
                std::cout << "Option 'VALUE' can not be empty" << std::endl;
                continue;
            }
            app.SetOption(opt.substr(0, pos), opt.substr(pos + 1));
        }

        else if( cmd == 'b' ) {
            app.SetBookmark(opt);
        }

        else if( cmd == 'c' ) {
            app.ApplyBookmark(opt);
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            app.Run();
        }

        else if( cmd == 'x' ) {
            std::vector<std::string> bookmarks = app.GetBookmarks();
            for( std::vector<std::string>::iterator it = bookmarks.begin(); it != bookmarks.end(); it++ ) {
                std::cout << (*it) << std::endl;
            }
        }

        else if( cmd == 'd' ) {
            std::cout << "Really delete bookmark '" << opt << "' ? [y|n] ";
            std::string answer;
            std::cin >> answer;
            while( std::cin.get() != '\n' ) {}
            if( answer == "y" ) {
                app.DeleteBookmark(opt);
            }
        }

        // Unknown command
        else if( cmd != 'q' ) {
            std::cout << "Unknown command !" << std::endl;
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
