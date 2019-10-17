enum InputSourceType {
    NO_INPUT_TYPE = 0,
    AUDIO_DEVICE = 1
};

enum ReceiverModeType {
    NO_RECEIVE_MODE = 0,
    CW = 1
};

enum DumpFileFormatType {
    PCM = 0,
    WAV = 1
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

    // First stage gain
    int FirstStageGain = 50;

    // Output volume
    int Volume = 200;

    // Dump pcm and audio
    bool DumpPcm = false;
    bool DumpAudio = false;
    DumpFileFormatType DumpFileFormat = WAV;
};

extern bool terminated;

#define BLOCKSIZE 4096
#define SAMPLERATE H_SAMPLE_RATE_48K

extern ConfigOptions configOptions;

extern HProcessor<int16_t>* processor;
extern HReader<int16_t>* inputReader;
extern HWriter<int16_t>* outputWriter;

extern HWriter<int16_t>* pcmWriter;
extern HWriter<int16_t>* audioWriter;
extern HSplitter<int16_t>* pcmSplitter;
extern HSplitter<int16_t>* audioSplitter;
extern HMute<int16_t>* pcmMute;
extern HMute<int16_t>* audioMute;

bool CreateCwReceiverChain(ConfigOptions* configOptions, HWriterConsumer<int16_t>* previous, HWriter<int16_t>* next);