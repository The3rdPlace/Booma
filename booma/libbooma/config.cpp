#include <stdlib.h>
#include <iostream>
#include <cstring>

#include "config.h"
#include "language.h"

ConfigOptions::ConfigOptions(int argc, char** argv) {

    // First pass: help or version
    for( int i = 1; i < argc; i++ ) {
        if( strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0 ) {
            std::cout << tr("Usage: booma-console [-option [parameter, ...]]") << std::endl;
            std::cout << std::endl;

            std::cout << tr("==[Information]==") << std::endl;
            std::cout << tr("Show this help and exit                  -h --help") << std::endl;
            std::cout << tr("Show version and exit                    --version") << std::endl;
            std::cout << std::endl;

            std::cout << tr("==[Options]==") << std::endl;
            std::cout << tr("Dump output audio as pcm to file         -a PCM") << std::endl;
            std::cout << tr("Dump output audio as wav to file         -a WAV") << std::endl;
            std::cout << tr("Select frequency (default 17.2KHz)       -f frequecy") << std::endl;
            std::cout << tr("Rf gain (default 30)                     -g gain") << std::endl;
            std::cout << tr("Select audio input source                -i AUDIO devicenumber ") << std::endl;
            std::cout << tr("Select CW receive mode (default)         -m CW") << std::endl;
            std::cout << tr("Select output device                     -o devicenumber") << std::endl;
            std::cout << tr("Dump raw input as pcm to file            -p PCM") << std::endl;
            std::cout << tr("Dump raw input as wav to file (default)  -p WAV") << std::endl;
            std::cout << tr("Receiver for remote input                -r address port") << std::endl;
            std::cout << tr("Server for remote input                  -s port") << std::endl;
            std::cout << tr("Output volume (default 200)              -v volume") << std::endl;
            std::cout << std::endl;

            std::cout << tr("==[Debugging]==") << std::endl;;
            std::cout << tr("Use sine generator as input              -is frequency") << std::endl;
            std::cout << tr("Select /dev/null as output device        -o -1") << std::endl;

            exit(0);
        }
        if( strcmp(argv[i], "--version" ) == 0 ) {
            std::cout << tr("Booma-console version 1.0.0 using Booma x and Hardt y") << std::endl;
            exit(0);
        }
    }

    // Second pass: config options
    for( int i = 0; i < argc; i++ ) {

        // Dump output as ... to file
        if( strcmp(argv[i], "-a") == 0 && i < argc + 1) {
            _dumpAudio = true;
            if( strcmp(argv[i + 1], "PCM") == 0 ) {
                _dumpFileFormat = PCM;
            } else if( strcmp(argv[i + 1], "WAV") == 0 ) {
               _dumpFileFormat = WAV;
            }
            i++;
            continue;
        }

        // Frequency
        if( strcmp(argv[i], "-f") == 0 && i < argc + 1) {
            _frequency = atoi(argv[i + 1]);
            i++;
            continue;
        }

        // Rf gain
        if( strcmp(argv[i], "-g") == 0 && i < argc + 1) {
            _rfGain = atoi(argv[i + 1]);
            i++;
            continue;
        }

        // Select input device
        if( strcmp(argv[i], "-i") == 0 && i < argc + 2) {
            if( strcmp(argv[i + 1], "AUDIO") == 0 ) {
                _inputSourceType = AUDIO_DEVICE;
                _inputAudioDevice = atoi(argv[i + 2]);
            }
            i += 2;
            continue;
        }

        // DEBUG: use sine generator as input
        if( strcmp(argv[i], "-is") == 0 && i < argc + 1) {
            _inputSourceType = SIGNAL_GENERATOR;
            _signalGeneratorFrequency = atoi(argv[i + 1]);
            i++;
            continue;
        }

        // Select receiver mode ...
        if( strcmp(argv[i], "-m") == 0 && i < argc + 1) {
            if( strcmp(argv[i + 1], "CW") == 0 ) {
                _receiverModeType = CW;
            }
            i++;
            continue;
        }

        // Select output device
        if( strcmp(argv[i], "-o") == 0 && i < argc + 1) {
            _outputAudioDevice = atoi(argv[i + 1]);
            i++;
            continue;
        }

        // Dump output audio as ...
        if( strcmp(argv[i], "-p") == 0) {
            _dumpPcm = true;
            if( strcmp(argv[i + 1], "PCM") == 0 ) {
                _dumpFileFormat = PCM;
            } else if( strcmp(argv[i + 1], "WAV") == 0 ) {
               _dumpFileFormat = WAV;
            }
            i++;
            continue;
        }

        // Server for remote input
        if( strcmp(argv[i], "-s") == 0 && argc < argc + 1) {
            _remotePort = atoi(argv[i + 1]);
            _useRemoteHead = true;
            i++;
            continue;
        }

        // Receiver for remote input
        if( strcmp(argv[i], "-r") == 0 && argc < argc + 2) {
            _remoteServer = argv[i + 1];
            _remotePort = atoi(argv[i + 2]);
            _isRemoteHead = true;
            i++;
            continue;
        }

        // Volume
        if( strcmp(argv[i], "-v") == 0 && i < argc + 1) {
            _volume = atoi(argv[i + 1]);
            i++;
            continue;
        }
    }

    // Check configuration for remote server/head
    if( _isRemoteHead && _remoteServer == NULL ) {
        std::cout << tr("Please select address of remote input with '-r address port'") << std::endl;
        exit(1);
    }
    if( _isRemoteHead && _remotePort == 0 ) {
        std::cout << tr("Please select port of remote input with '-r address port'") << std::endl;
        exit(1);
    }
    if( _useRemoteHead && _remotePort == 0 ) {
        std::cout << tr("Please select port for the remote input server with '-s port'") << std::endl;
        exit(1);
    }

    // Check that the required minimum of settings has been provided
    if( _useRemoteHead == false ) {
        if( _receiverModeType == NO_RECEIVE_MODE ) {
            std::cout << tr("Please select the receive mode with '-m [CW]'") << std::endl;
            exit(1);
        }
    }
    if( _isRemoteHead == false ) {
        if( _inputSourceType == NO_INPUT_TYPE ) {
            std::cout << tr("Please select the input type with '-i [AUDIO] devicenumber'") << std::endl;
            exit(1);
        }
        if( _inputSourceType == AUDIO_DEVICE && _inputAudioDevice < 0 ) {
            std::cout << tr("Please select the input audio device with '-i [AUDIO] devicenumber'") << std::endl;
            exit(1);
        }
        if( _inputSourceType == SIGNAL_GENERATOR && _signalGeneratorFrequency < 0 ) {
            std::cout << tr("Please select the input signal generator frequency with '-is frequency'") << std::endl;
            exit(1);
        }
    }
}
