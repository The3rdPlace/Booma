#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#include "booma.h"

enum InputSourceType {
    NONE = 0,
    AUDIO_DEVICE = 1
};

enum ReceiverModeType {
    NONE = 0,
    CW = 1
};

struct ConfigOptions {

    // Output audio device
    int outputAudioDevice = -1;

    // Input type
    InputSourceType inputSourceType = AUDIO;

    // InputSourceType = AUDIO_DEVICE
    int inputAudioDevice = -1;

    // Initial or last used frequency
    long int frequency = 17200;

    // Receiver mode
    ReceiverModeType receiverModeType = CW;

    // Beattone frequency for CW operation
    int BeattoneFrequency = 1200;
};

std::string tr(std::string source) {
    return source;
}

bool parseArgs(int argc, char** argv) {
    ConfigOptions configOptions;

    // First pass: help or version
    for( int i = 1; i < argc; i++ ) {
        if( strcmp(argv[i], "-h") == 0 ) {
            std::cout << tr("Usage: booma-console [-option [parameter, ...]]") << std::endl;
            std::cout << std::endl;
            std::cout << tr("Options:") << std::endl;
            std::cout << tr("Select output device     -o devicenumber") << std::endl;
            std::cout << tr("Use audio input source   -i AUDIO devicenumber ") << std::endl;
            std::cout << tr("Select CW receive mode   -m CW") << std::endl;
            exit(0);
        }
        if( stdcmp(argv[i], "-v" ) == 0 ) {
            std::cout << tr("Booma-console version 1.0.0 using Booma x and Hardt y") << std::endl;
            exit(0);
        }
    }

    // Second pass: config options
    for( int i = 0; i < argc; i++ ) {

    }

    // Check that the required minimum of settings has been provided
    if( configOptions.outputAudioDevice < 0 ) {
        std::cout << tr("Please select the output audio device with '-o devicenumber'") << std::endl;
        exit(1);
    }
    if( configOptions.inputSourceType == NONE ) {
        std::cout << tr("Please select the input type with '-i [AUDIO] devicenumber'") << std::endl;
        exit(1);
    }
    if( configOptions.inputSourceType == AUDIO && configOptions.inputAudioDevice < 0 ) {
        std::cout << tr("Please select the input audio device with '-i [AUDIO] devicenumber'") << std::endl;
        exit(1);
    }
    if( configOptions.receiverModeType == NONE ) {
        std::cout << tr("Please select the receive mode with '-m [CW]'") << std::endl;
        exit(1);
    }
}

int main(int argc, char** argv) {
	BoomaInit(true);

	return 0;
}
