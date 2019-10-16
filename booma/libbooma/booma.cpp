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
            std::cout << tr("Select output device                -o devicenumber") << std::endl;
            std::cout << tr("Use audio input source              -i AUDIO devicenumber ") << std::endl;
            std::cout << tr("Select CW receive mode (default)    -m CW") << std::endl;
            std::cout << tr("Select frequency (default 17.2KHz)  -f frequecy") << std::endl;
            std::cout << tr("Server for remote input             -s port") << std::endl;
            std::cout << tr("Receiver for remote input           -r address port") << std::endl;
            std::cout << tr("First stage gain (default 30)       -g gain") << std::endl;
            std::cout << tr("Output volume (default 200)         -v volume") << std::endl;
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
            configOptions->OutputAudioDevice = atoi(argv[i + 1]);
            i++;
            continue;
        }

        if( strcmp(argv[i], "-i") == 0 && i < argc + 2) {
            if( strcmp(argv[i + 1], "AUDIO") == 0 ) {
                configOptions->InputSourceType = AUDIO_DEVICE;
                configOptions->InputAudioDevice = atoi(argv[i + 2]);
            }
            i += 2;
            continue;
        }

        if( strcmp(argv[i], "-m") == 0 && i < argc + 1) {
            if( strcmp(argv[i + 1], "CW") == 0 ) {
                configOptions->ReceiverModeType = CW;
            }
            i++;
            continue;
        }

        if( strcmp(argv[i], "-f") == 0 && i < argc + 1) {
            configOptions->Frequency = atoi(argv[i + 1]);
            i++;
            continue;
        }

        if( strcmp(argv[i], "-s") == 0 && argc < argc + 1) {
            configOptions->RemotePort = atoi(argv[i + 1]);
            configOptions->UseRemoteHead = true;
            i++;
            continue;
        }

        if( strcmp(argv[i], "-r") == 0 && argc < argc + 2) {
            configOptions->RemoteServer = argv[i + 1];
            configOptions->RemotePort = atoi(argv[i + 2]);
            configOptions->IsRemoteHead = true;
            i++;
            continue;
        }

        if( strcmp(argv[i], "-g") == 0 && i < argc + 1) {
            configOptions->FirstStageGain = atoi(argv[i + 1]);
            i++;
            continue;
        }

        if( strcmp(argv[i], "-v") == 0 && i < argc + 1) {
            configOptions->Volume = atoi(argv[i + 1]);
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
        if( configOptions->OutputAudioDevice < 0 ) {
            std::cout << tr("Please select the output audio device with '-o devicenumber'") << std::endl;
            exit(1);
        }
        if( configOptions->ReceiverModeType == NO_RECEIVE_MODE ) {
            std::cout << tr("Please select the receive mode with '-m [CW]'") << std::endl;
            exit(1);
        }
    }
    if( configOptions->IsRemoteHead == false ) {
        if( configOptions->InputSourceType == NO_INPUT_TYPE ) {
            std::cout << tr("Please select the input type with '-i [AUDIO] devicenumber'") << std::endl;
            exit(1);
        }
        if( configOptions->InputSourceType == AUDIO_DEVICE && configOptions->InputAudioDevice < 0 ) {
            std::cout << tr("Please select the input audio device with '-i [AUDIO] devicenumber'") << std::endl;
            exit(1);
        }
    }
}

ConfigOptions configOptions;

bool terminated = false;

HProcessor<int16_t>* processor;
HReader<int16_t>* inputReader;
HWriter<int16_t>* outputWriter;

ConfigOptions* BoomaInit(int argc, char** argv, bool verbose) {

    // Initialize the Hardt toolkit.
    // Set the last argument to 'true' to enable verbose output instead of logging to a local file
    HInit(std::string("Booma"), verbose);

    // Parse input arguments
    parseArgs(argc, argv, &configOptions);

    // Show library name and and Hardt version.
    std::cout << "booma: using Hardt " + getversion() << std::endl;

    // Setup input
    BoomaSetInput(&configOptions);

    // Setup output
    BoomaSetOutput(&configOptions);

    // Setup receiver chain for selected mode
    BoomaBuildReceiverChain(&configOptions);

    // Return the configured options
    return &configOptions;
}

bool BoomaSetInput(ConfigOptions* configOptions) {

    // If we are a remote head, then initialize a network processor
    if( configOptions->IsRemoteHead ) {
        HLog("Initializing network processor with remote input at %s:%d", configOptions->RemoteServer, configOptions->RemotePort);
        processor = new HNetworkProcessor<int16_t>(configOptions->RemoteServer, configOptions->RemotePort, BLOCKSIZE, &terminated);
        return true;
    }

    // If we are a server for a remote head, then initialize the input and a network processor
    if( configOptions->UseRemoteHead) {
        HLog("Initializing network processor with local audio input device %d", configOptions->InputAudioDevice);
        inputReader = new HSoundcardReader<int16_t>(configOptions->InputAudioDevice, SAMPLERATE, 1, H_SAMPLE_FORMAT_INT_16, BLOCKSIZE);
        processor = new HNetworkProcessor<int16_t>(configOptions->RemotePort, inputReader, BLOCKSIZE, &terminated);
        return true;
    }

    // Otherwise configure a local input
    switch( configOptions->InputSourceType ) {
        case AUDIO_DEVICE:
            HLog("Initializing audio input device %d", configOptions->InputAudioDevice);
            inputReader = new HSoundcardReader<int16_t>(configOptions->InputAudioDevice, SAMPLERATE, 1, H_SAMPLE_FORMAT_INT_16, BLOCKSIZE);
            processor = new HStreamProcessor<int16_t>(inputReader, BLOCKSIZE, &terminated);
            return true;
        default:
            std::cout << "No input source type defined" << std::endl;
            return false;
    }
}

bool BoomaSetOutput(ConfigOptions* configOptions) {

    // If we have a remote head, then do nothing
    if( configOptions->UseRemoteHead ) {
        HLog("Using remote head, no local output");
        return true;
    }

    // Otherwise initialize the audio output
    HLog("Initializing audio output device");
    outputWriter = new HSoundcardWriter<int16_t>(configOptions->OutputAudioDevice, SAMPLERATE, 1, H_SAMPLE_FORMAT_INT_16, BLOCKSIZE);
    //outputWriter = new HWavWriter<int16_t>("/Users/henrik/Git/Booma/build/booma_out.wav", H_SAMPLE_FORMAT_INT_16, 1, SAMPLERATE);

    return true;
}

bool BoomaBuildReceiverChain(ConfigOptions* configOptions) {

    // If we have a remote head, then do nothing
    if( configOptions->UseRemoteHead ) {
    HLog("Using remote head, no local receiver chain");
        return true;
    }

    HLog("Creating receiver chain for selected rx mode");
    switch( configOptions->ReceiverModeType ) {
        case CW:
            return CreateCwReceiverChain(configOptions);
        default:
            std::cout << "Unknown receiver type defined" << std::endl;
            return false;
    }
}

void BoomaRun() {
    //processor->Run(50);
}