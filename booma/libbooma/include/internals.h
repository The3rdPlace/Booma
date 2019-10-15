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
    int OutputAudioDevice = -1;

    // Input type
    InputSourceType InputSourceType = AUDIO_DEVICE;

    // InputSourceType = AUDIO_DEVICE
    int InputAudioDevice = -1;

    // Initial or last used frequency
    long int Frequency = 17200;

    // Receiver mode
    ReceiverModeType ReceiverModeType = CW;

    // Beattone frequency for CW operation
    int BeattoneFrequency = 1200;

    // Remote head configuration
    bool IsRemoteHead = false;
    char* RemoteServer = NULL;
    int RemotePort = 0;
    bool UseRemoteHead = false;
};

extern bool terminated;

#define BLOCKSIZE 4096
#define SAMPLERATE H_SAMPLE_RATE_48K

extern ConfigOptions configOptions;

extern HProcessor<int16_t>* processor;
extern HReader<int16_t>* inputReader;
extern HWriter<int16_t>* outputWriter;

bool CreateCwReceiverChain(ConfigOptions* configOptions);