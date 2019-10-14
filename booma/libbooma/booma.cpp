#include <stdlib.h>
#include <iostream>

#include <hardtapi.h>

#include <booma.h>
#include "internals.h"


std::string tr(std::string source) {
    return source;
}

void parseArgs(int argc, char** argv, ConfigOptions *configOptions) {

    // First pass: help or version
    for( int i = 1; i < argc; i++ ) {
        if( strcmp(argv[i], "-h") == 0 ) {
            std::cout << tr("Usage: booma-console [-option [parameter, ...]]") << std::endl;
            std::cout << std::endl;
            std::cout << tr("Options:") << std::endl;
            std::cout << tr("Select output device         -o devicenumber") << std::endl;
            std::cout << tr("Use audio input source       -i AUDIO devicenumber ") << std::endl;
            std::cout << tr("Select CW receive mode       -m CW") << std::endl;
            std::cout << tr("Select frequency             -f frequecy") << std::endl;
            std::cout << tr("Server for remote input      -s port");
            std::cout << tr("Receiver for remote inpu     -r address port");
            exit(0);
        }
        if( strcmp(argv[i], "-v" ) == 0 ) {
            std::cout << tr("Booma-console version 1.0.0 using Booma x and Hardt y") << std::endl;
            exit(0);
        }
    }

    // Second pass: config options
    for( int i = 0; i < argc; i++ ) {
        if( strcmp(argv[i], "-o") == 0 && i < argc + 1) {
            configOptions->outputAudioDevice = atoi(argv[i + 1]);
            i++;
            continue;
        }

        if( strcmp(argv[i], "-i") == 0 && i < argc + 2) {
            if( strcmp(argv[i + 1], "AUDIO") == 0 ) {
                configOptions->inputSourceType = AUDIO_DEVICE;
                configOptions->inputAudioDevice = atoi(argv[i + 2]);
            }
            i += 2;
            continue;
        }

        if( strcmp(argv[i], "-m") == 0 && i < argc + 1) {
            if( strcmp(argv[i + 1], "CW") == 0 ) {
                configOptions->receiverModeType = CW;
            }
            i++;
            continue;
        }

        if( strcmp(argv[i], "-f") == 0 && i < argc + 1) {
            configOptions->frequency = atoi(argv[i + 1]);
            i++;
            continue;
        }

        if( strcmp(argv[i], "-s") == 0 && argc < argc + 1) {
            configOptions->RemotePort = atoi(argv[i + 1]);
            i++;
            continue;
        }

        if( strcmp(argv[i], "-r") == 0 && argc < argc + 2) {
            configOptions->RemoteServer = argv[i + 1];
            configOptions->RemotePort = atoi(argv[i + 2]);
            i++;
            continue;
        }
    }

    // Check configuration for remote server/head
    if( configOptions->IsRemoteHead && configOptions->RemoteServer == NULL ) {
        std::cout << tr("Please select address of remote input with '-r address port'") << std::endl;
        exit(1);
    }
    if( configOptions->IsRemoteHead && configOptions->RemotePort == 0 ) {
        std::cout << tr("Please select port of remote input with '-r address port'") << std::endl;
        exit(1);
    }
    if( configOptions->UseRemoteHead && configOptions->RemotePort == 0 ) {
        std::cout << tr("Please select port for the remote input server with '-s port'") << std::endl;
        exit(1);
    }

    // Check that the required minimum of settings has been provided
    if( configOptions->UseRemoteHead == false ) {
        if( configOptions->outputAudioDevice < 0 ) {
            std::cout << tr("Please select the output audio device with '-o devicenumber'") << std::endl;
            exit(1);
        }
        if( configOptions->receiverModeType == NO_RECEIVE_MODE ) {
            std::cout << tr("Please select the receive mode with '-m [CW]'") << std::endl;
            exit(1);
        }
    }
    if( configOptions->IsRemoteHead == false ) {
        if( configOptions->inputSourceType == NO_INPUT_TYPE ) {
            std::cout << tr("Please select the input type with '-i [AUDIO] devicenumber'") << std::endl;
            exit(1);
        }
        if( configOptions->inputSourceType == AUDIO_DEVICE && configOptions->inputAudioDevice < 0 ) {
            std::cout << tr("Please select the input audio device with '-i [AUDIO] devicenumber'") << std::endl;
            exit(1);
        }
    }
}

ConfigOptions configOptions;

HProcessor<int16_t>* processor;
HReader<int16_t>* inputReader;
HWriterint16_t>* outputWriter;

void BoomaInit(int argc, char** argv, bool verbose) {

    // Initialize the Hardt toolkit.
    // Set the last argument to 'true' to enable verbose output instead of logging to a local file
    HInit(std::string("Booma"), verbose);

    // Parse input arguments
    parseArgs(argc, argv, &configOptions);

    // Show library name and and Hardt version.
    std::cout << "booma: using Hardt " + getversion() << std::endl;

    // Setup input
    BoomaSetInput();

    // Setup output
    BoomaSetOutput(&configOptions);

    // Setup receiver chain for selected mode
    BoomaBuildReceiverChain(&configOptions);
}

bool BoomaSetInput(ConfigOptions* configOptions) {
    // If remote head, initialize network rocessor
    // else...

    switch( configOptions.inputSourceType ) {
        case AUDIO_DEVICE:
            HLog("Initializing audio input device %d", configOptions.inputAudioDevice);
            inputReader = new HSoundcardReader<int16_t>(...);
            processor = new HStreamProcessor<int16_t>(inputReader, ...); // or networkprocessor
            return true;
        default:
            std::cout << "No input source type defined" << std::endl;
            exit(1);
    }
}

bool BoomaSetOutput(ConfigOptions* configOptions) {
    HLog("Initializing audio output device");
    outputWriter = new HSoundcardWriter<int16_t>();
}

bool BoomaBuildReceiverChain(ConfigOptions* configOptions) {
    // processor::SetWriter('first-writer')
    // 'last-writer'->SetWriter(outputWriter.Consumer());
}

void BoomaRun() {
    // processor::run()
}