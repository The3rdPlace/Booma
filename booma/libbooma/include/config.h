#ifndef __CONFIG_H
#define __CONFIG_H

#include <hardtapi.h>

/** Type of input device */
enum InputSourceType {
    NO_INPUT_TYPE = 0,
    AUDIO_DEVICE = 1,
    SIGNAL_GENERATOR = 2,
    PCM_FILE = 3,
    WAV_FILE = 4,
    SILENCE = 5,
    NETWORK = 6
};

/** Type of receiver */
enum ReceiverModeType {
    NO_RECEIVE_MODE = 0,
    CW = 1,
    AURORAL = 2,
    CW2 = 3
};

/** Format of the dump file */
enum DumpFileFormatType {
    PCM = 0,
    WAV = 1
};

#define CONFIGNAME "config.ini"

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
        ReceiverModeType _receiverModeType = CW2;

        // Remote head configuration
        bool _isRemoteHead = false;
        std::string _remoteServer;
        int _remoteDataPort = 0;
        int _remoteCommandPort = 0;
        bool _useRemoteHead = false;

        // First stage gain
        int _rfGain = 5;

        // Output volume
        int _volume = 50;

        // Dump pcm and audio
        bool _dumpRf = false;
        bool _dumpAudio = false;
        DumpFileFormatType _dumpRfFileFormat = PCM;
        DumpFileFormatType _dumpAudioFileFormat = WAV;

        // Scheduled start and stop
        HTimer _schedule;

        // Debug options
        int _signalGeneratorFrequency = -1;
        std::string _pcmFile = "";
        std::string _wavFile = "";

        void PrintUsage();
        void PrintCards();

        bool ReadStoredConfig(std::string configname);
        void WriteStoredConfig(std::string configname);
        void RemoveStoredConfig(std::string configname);
        std::map<std::string, std::string> ReadStoredReceiverOptions(std::string optionsString);
        std::string WriteStoredReceiverOptions(std::map<std::string, std::string> options);
        std::map<std::string, std::map<std::string, std::string>> ReadStoredReceiverOptionsFor(std::string optionsForString);
        std::string WriteStoredReceiverOptionsFor(std::map<std::string,std::map<std::string, std::string>> options);

        bool _enableProbes = false;

        int _reservedBuffers = 200;

        std::map<std::string, std::string> _receiverOptions;
        std::map<std::string, std::map<std::string, std::string>> _receiverOptionsFor;

    public:

        ConfigOptions(std::string appName, std::string appVersion, int argc, char** argv);

        ~ConfigOptions();

        static bool IsVerbose(int argc, char** argv);

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

        int GetRemoteDataPort() {
            return _remoteDataPort;
        }

        int GetRemoteCommandPort() {
            return _remoteCommandPort;
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

        DumpFileFormatType GetDumpRfFileFormat() {
            return _dumpRfFileFormat;
        }

        DumpFileFormatType GetDumpAudioFileFormat() {
            return _dumpAudioFileFormat;
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

        std::string GetWavFile() {
            return _wavFile;
        }

        bool GetEnableProbes() {
            return _enableProbes;
        }

        int GetReservedBuffers() {
            return _reservedBuffers;
        }

        bool GetEnableBuffers() {
            return _reservedBuffers > 0;
        }

        HTimer GetSchedule() {
            return _schedule;
        }

        std::map<std::string, std::string>* GetReceiverOptions() {
            return &_receiverOptions;
        }

        void SetReceiverOptionsFor(std::string receiver, std::map<std::string, std::string> options);

        std::map<std::string, std::string> GetReceiverOptionsFor(std::string receiver);

        void WriteBookmark(std::string name);

        bool ReadBookmark(std::string name);

        void DeleteBookmark(std::string name);

        std::vector<std::string> ListBookmarks();
    };

#endif
