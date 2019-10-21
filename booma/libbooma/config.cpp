#include <stdlib.h>
#include <iostream>
#include <cstring>

#include "config.h"
#include "language.h"

ConfigOptions::ConfigOptions(int argc, char** argv) {

    // First pass: help or version
    for( int i = 1; i < argc; i++ ) {
        if( strcmp(argv[i], "-h") == 0 ) {
            std::cout << tr("Usage: booma-console [-option [parameter, ...]]") << std::endl;
            std::cout << std::endl;
            std::cout << tr("Options:") << std::endl;
            std::cout << tr("Select output device                -o devicenumber") << std::endl;
            std::cout << tr("Use audio input source              -i AUDIO devicenumber ") << std::endl;
            std::cout << tr("Dump output audio pcm to file       -a PCM") << std::endl;
            std::cout << tr("Dump output audio to wav file       -a WAV") << std::endl;
            std::cout << tr("Select CW receive mode (default)    -m CW") << std::endl;
            std::cout << tr("Select frequency (default 17.2KHz)  -f frequecy") << std::endl;
            std::cout << tr("Server for remote input             -s port") << std::endl;
            std::cout << tr("Receiver for remote input           -r address port") << std::endl;
            std::cout << tr("First stage gain (default 30)       -g gain") << std::endl;
            std::cout << tr("Output volume (default 200)         -v volume") << std::endl;
            std::cout << tr("Dump raw input pcm to file          -p PCM") << std::endl;
            std::cout << tr("Dump input pcm to wav file          -p WAV") << std::endl;
            std::cout << tr("Show version and exit               --version") << std::endl;
            exit(0);
        }
        if( strcmp(argv[i], "--version" ) == 0 ) {
            std::cout << tr("Booma-console version 1.0.0 using Booma x and Hardt y") << std::endl;
            exit(0);
        }
    }

    // Second pass: config options
    for( int i = 0; i < argc; i++ ) {
        if( strcmp(argv[i], "-o") == 0 && i < argc + 1) {
            _outputAudioDevice = atoi(argv[i + 1]);
            i++;
            continue;
        }

        if( strcmp(argv[i], "-i") == 0 && i < argc + 2) {
            if( strcmp(argv[i + 1], "AUDIO") == 0 ) {
                _inputSourceType = AUDIO_DEVICE;
                _inputAudioDevice = atoi(argv[i + 2]);
            }
            i += 2;
            continue;
        }

        if( strcmp(argv[i], "-m") == 0 && i < argc + 1) {
            if( strcmp(argv[i + 1], "CW") == 0 ) {
                _receiverModeType = CW;
            }
            i++;
            continue;
        }

        if( strcmp(argv[i], "-f") == 0 && i < argc + 1) {
            _frequency = atoi(argv[i + 1]);
            i++;
            continue;
        }

        if( strcmp(argv[i], "-s") == 0 && argc < argc + 1) {
            _remotePort = atoi(argv[i + 1]);
            _useRemoteHead = true;
            i++;
            continue;
        }

        if( strcmp(argv[i], "-r") == 0 && argc < argc + 2) {
            _remoteServer = argv[i + 1];
            _remotePort = atoi(argv[i + 2]);
            _isRemoteHead = true;
            i++;
            continue;
        }

        if( strcmp(argv[i], "-g") == 0 && i < argc + 1) {
            _firstStageGain = atoi(argv[i + 1]);
            i++;
            continue;
        }

        if( strcmp(argv[i], "-v") == 0 && i < argc + 1) {
            _volume = atoi(argv[i + 1]);
            i++;
            continue;
        }

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
        if( _outputAudioDevice < 0 ) {
            std::cout << tr("Please select the output audio device with '-o devicenumber'") << std::endl;
            exit(1);
        }
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
    }
}
