#ifndef __CONFIG_H
#define __CONFIG_H

/** Type of input device */
enum InputSourceType {
    NO_INPUT_TYPE = 0,
    AUDIO_DEVICE = 1
};

/** Type of receiver */
enum ReceiverModeType {
    NO_RECEIVE_MODE = 0,
    CW = 1
};

/** Format of the dump file */
enum DumpFileFormatType {
    PCM = 0,
    WAV = 1
};

/** Configuration object */
class ConfigOptions {

    private:

        // Output audio device
        int _outputAudioDevice = -1;

        // Input type
        InputSourceType _inputSourceType = AUDIO_DEVICE;

        // InputSourceType = AUDIO_DEVICE
        int _inputAudioDevice = -1;

        // Initial or last used frequency
        long int _frequency = 17200;

        // Receiver mode
        ReceiverModeType _receiverModeType = CW;

        // Remote head configuration
        bool _isRemoteHead = false;
        char* _remoteServer = NULL;
        int _remotePort = 0;
        bool _useRemoteHead = false;

        // First stage gain
        int _firstStageGain = 20;

        // Output volume
        int _volume = 200;

        // Dump pcm and audio
        bool _dumpPcm = false;
        bool _dumpAudio = false;
        DumpFileFormatType _dumpFileFormat = WAV;

    public:

        ConfigOptions(int argc, char** argv);

        int getOutputAudioDevice() {
            return _outputAudioDevice;
        }

        InputSourceType getInputSourceType() {
            return _inputSourceType;
        }

        int getInputAudioDevice() {
            return _inputAudioDevice;
        }

        long int getFrequency() {
            return _frequency;
        }

        void setFrequency(long int frequency) {
            _frequency = frequency;
        }

        ReceiverModeType getReceiverModeType() {
            return _receiverModeType;
        }

        bool getIsRemoteHead() {
            return _isRemoteHead;
        }

        char* getRemoteServer() {
            return _remoteServer;
        }

        int getRemotePort() {
            return _remotePort;
        }

        bool getUseRemoteHead() {
            return _useRemoteHead;
        }

        int getFirstStageGain() {
            return _firstStageGain;
        }

        int getVolume() {
            return _volume;
        }

        bool getDumpPcm() {
            return _dumpPcm;
        }

        bool getDumpAudio() {
            return _dumpAudio;
        }

        DumpFileFormatType getDumpFileFormat() {
            return _dumpFileFormat;
        }
};

#endif