#ifndef __CONFIG_H
#define __CONFIG_H

#include <hardtapi.h>

/** Type of input device */
enum InputSourceType {
    NO_INPUT_TYPE = 0,
    AUDIO_DEVICE = 1,
    SIGNAL_GENERATOR = 2,
    PCM_FILE = 3,
    SILENCE = 4
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

        // Samplerate
        int _sampleRate = H_SAMPLE_RATE_48K;

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
        std::string _remoteServer;
        int _remotePort = 1720;
        bool _useRemoteHead = false;

        // First stage gain
        int _rfGain = 20;

        // Output volume
        int _volume = 200;

        // Dump pcm and audio
        bool _dumpRf = false;
        bool _dumpAudio = false;
        DumpFileFormatType _dumpFileFormat = WAV;

        // Debug options
        int _signalGeneratorFrequency = -1;
        std::string _pcmFile = "";

        void PrintUsage();
        void PrintCards();

        bool ReadStoredConfig();
        void SaveStoredConfig();

    public:

        ConfigOptions(std::string appName, std::string appVersion, int argc, char** argv);

        ~ConfigOptions();

        int GetOutputAudioDevice() {
            return _outputAudioDevice;
        }

        InputSourceType GetInputSourceType() {
            return _inputSourceType;
        }

        int GetInputAudioDevice() {
            return _inputAudioDevice;
        }

        long int GetFrequency() {
            return _frequency;
        }

        void SetFrequency(long int frequency) {
            _frequency = frequency;
        }

        ReceiverModeType GetReceiverModeType() {
            return _receiverModeType;
        }

        void SetReceiverModeType(ReceiverModeType receiverModeType) {
            _receiverModeType = receiverModeType;
        }

        bool GetIsRemoteHead() {
            return _isRemoteHead;
        }

        std::string GetRemoteServer() {
            return _remoteServer;
        }

        int GetRemotePort() {
            return _remotePort;
        }

        bool GetUseRemoteHead() {
            return _useRemoteHead;
        }

        int GetRfGain() {
            return _rfGain;
        }

        void SetRfGain(int gain) {
            _rfGain = gain;
        }

        int GetVolume() {
            return _volume;
        }

        void SetVolume(int volume) {
            _volume = volume;
        }

        bool GetDumpRf() {
            return _dumpRf;
        }

        void SetDumpRf(bool enabled) {
            _dumpRf = enabled;
        }

        bool GetDumpAudio() {
            return _dumpAudio;
        }

        void SetDumpAudio(bool enabled) {
            _dumpAudio = enabled;
        }

        DumpFileFormatType GetDumpFileFormat() {
            return _dumpFileFormat;
        }

        int GetSignalGeneratorFrequency() {
            return _signalGeneratorFrequency;
        }

        int GetSampleRate() {
            return _sampleRate;
        }

        void SetSampleRate(int sampleRate) {
            _sampleRate = sampleRate;
        }

        std::string GetPcmFile() {
            return _pcmFile;
        }
};

#endif
