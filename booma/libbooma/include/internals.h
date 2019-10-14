enum InputSourceType {
    NO_INPUT_TYPE = 0,
    AUDIO_DEVICE = 1
};

enum ReceiverModeType {
    NO_RECEIVE_MODE = 0,
    CW = 1
};

struct ConfigOptions {

    // Output audio device
    int outputAudioDevice = -1;

    // Input type
    InputSourceType inputSourceType = AUDIO_DEVICE;

    // InputSourceType = AUDIO_DEVICE
    int inputAudioDevice = -1;

    // Initial or last used frequency
    long int frequency = 17200;

    // Receiver mode
    ReceiverModeType receiverModeType = CW;

    // Beattone frequency for CW operation
    int BeattoneFrequency = 1200;

    // Remote head configuration
    bool IsRemoteHead = false;
    char* RemoteServer = NULL;
    int RemotePort = 0;
    bool UseRemoteHead = false;
};

extern ConfigOptions configOptions;

extern HProcessor<int16_t>* processor;
extern HReader<int16_t>* inputReader;
extern HWriterint16_t>* outputWriter;