#ifndef __CONFIGOPTIONVALUES_H
#define __CONFIGOPTIONVALUES_H

/** Type of input device */
enum InputSourceType {
    NO_INPUT_SOURCE_TYPE = 0,
    AUDIO_DEVICE = 1,
    SIGNAL_GENERATOR = 2,
    PCM_FILE = 3,
    WAV_FILE = 4,
    SILENCE = 5,
    NETWORK = 6,
    RTLSDR = 7
};

/** Type of input data received from the input device */
enum InputSourceDataType {
    NO_INPUT_SOURCE_DATA_TYPE = 0,
    REAL_INPUT_SOURCE_DATA_TYPE = 1,
    IQ_INPUT_SOURCE_DATA_TYPE = 2,
    I_INPUT_SOURCE_DATA_TYPE = 3,
    Q_INPUT_SOURCE_DATA_TYPE = 4
};

/** Type of receiver */
enum ReceiverModeType {
    NO_RECEIVE_MODE = 0,
    CW = 1,
    AURORAL = 2,
    AM = 3,
    SSB = 4
};

/** Format of the dump file */
enum DumpFileFormatType {
    PCM = 0,
    WAV = 1
};

struct ConfigOptionValues {

    // Samplerates
    int _inputSampleRate = H_SAMPLE_RATE_48K;
    int _outputSampleRate = H_SAMPLE_RATE_48K;

    // Output audio device
    int _outputAudioDevice = 0;
    std::string _outputFilename = "";

    // Input device- and datatype
    InputSourceType _inputSourceType = NO_INPUT_SOURCE_TYPE;
    InputSourceType _originalInputSourceType = NO_INPUT_SOURCE_TYPE;
    InputSourceDataType _inputSourceDataType = NO_INPUT_SOURCE_DATA_TYPE;

    // Input device
    int _inputDevice = 0;

    // RTL-SDR and IQ/I/Q specific settings
    long int _rtlsdrAdjust = 0;

    // Initial or last used frequency
    long int _frequency = 17200;

    // When used with up/down converters, shift frequencies with this amount
    long int _shift = 0;

    // Receiver mode
    ReceiverModeType _receiverModeType = CW;

    // Remote head configuration
    bool _isRemoteHead = false;
    std::string _remoteServer;
    int _remoteDataPort = 0;
    int _remoteCommandPort = 0;
    bool _useRemoteHead = false;

    // First stage gain (default 0 = auto)
    int _rfGain = 0;
    int _rfAgcLevel = 500;

    // Output volume
    int _volume = 20;

    // Dump pcm and audio
    bool _dumpRf = false;
    bool _dumpAudio = false;
    DumpFileFormatType _dumpRfFileFormat = PCM;
    DumpFileFormatType _dumpAudioFileFormat = WAV;
    std::string _dumpFileSuffix = "";

    // Scheduled start and stop
    HTimer _schedule;

    // Debug options
    int _signalGeneratorFrequency = -1;
    std::string _pcmFile = "";
    std::string _wavFile = "";
    bool _frequencyAlign = false;
    int _frequencyAlignVolume = 500;

    bool _enableProbes = false;

    int _reservedBuffers = 200;

    std::map<std::string, std::string> _receiverOptions;
    std::map<std::string, std::map<std::string, std::string>> _receiverOptionsFor;

    // Decimation settings
    int _decimatorGain = 0; // = auto
    int _decimatorAgcLevel = 1000;

    // Internal values, usually left at standard values.
    // These are not settable while running, so they must be set when starting the application
    int _rtlsdrOffset = 6000;
    int _rtlsdrCorrection = 0;
    int _rtlsdrCorrectionFactor = 0;
    int _rtlsdrGain = 0;
    int _firFilterSize = 51;
    int _inputFilterWidth = 3000;
};

#endif