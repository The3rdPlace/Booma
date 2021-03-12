#ifndef __CONFIG_H
#define __CONFIG_H

#include <stdlib.h>
#include <iostream>
#include <cstring>

#include <hardtapi.h>
#include "channel.h"

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

#define CONFIGNAME "config.ini"

/** Configuration object */
class ConfigOptions {

    private:

        // Samplerates
        int _inputSampleRate = H_SAMPLE_RATE_48K;
        int _outputSampleRate = H_SAMPLE_RATE_48K;

        // Output audio device
        int _outputAudioDevice = -1;
        std::string _outputFilename = "";

        // Input device- and datatype
        InputSourceType _inputSourceType = NO_INPUT_SOURCE_TYPE;
        InputSourceType _originalInputSourceType = NO_INPUT_SOURCE_TYPE;
        InputSourceDataType _inputSourceDataType = NO_INPUT_SOURCE_DATA_TYPE;

        // Input device
        int _inputDevice = -1;

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
        int _rfAgcLevel = 1000;

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

        // Memory channels
        std::vector<Channel*> _channels;

        void PrintUsage(bool showSecretSettings = false);
        void PrintAudioDevices();
        void PrintRtlsdrDevices();
        std::string GetAudioDevice(int device);
        std::string GetRtlsdrDevice(int device);

        bool ReadStoredConfig(std::string configname);
        void WriteStoredConfig(std::string configname);
        void RemoveStoredConfig(std::string configname);
        std::map<std::string, std::string> ReadStoredReceiverOptions(std::string optionsString);
        std::string WriteStoredReceiverOptions(std::map<std::string, std::string> options);
        std::map<std::string, std::map<std::string, std::string>> ReadStoredReceiverOptionsFor(std::string optionsForString);
        std::string WriteStoredReceiverOptionsFor(std::map<std::string,std::map<std::string, std::string>> options);
        std::vector<Channel*> ReadChannels(std::string configname, std::string channels);
        std::vector<Channel*> ReadPersistentChannels(std::string configname);
        std::string WriteChannels(std::string configname, std::vector<Channel*> channels);
        void DumpConfigInfo();

        bool _enableProbes = false;

        int _reservedBuffers = 200;

        std::map<std::string, std::string> _receiverOptions;
        std::map<std::string, std::map<std::string, std::string>> _receiverOptionsFor;

        // Decimation settings
        int _decimatorGain = 0; // = auto
        int _decimatorAgcLevel = 2000;

        // Internal values, usually left at standard values.
        // These are not settable while running, so they must be set when starting the application
        int _rtlsdrOffset = 6000;
        int _rtlsdrCorrection = 0;
        int _rtlsdrCorrectionFactor = 0;
        int _firFilterSize = 51;

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

        InputSourceType GetOriginalInputSourceType() {
            return _originalInputSourceType == NO_INPUT_SOURCE_TYPE
            ? _inputSourceType
            : _originalInputSourceType;
        }

        InputSourceDataType GetInputSourceDataType() {
            return _inputSourceDataType;
        }

        int GetInputDevice() {
            return _inputDevice;
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

        int GetRfAgcLevel() {
            return _rfAgcLevel;
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

        int GetInputSampleRate() {
            return _inputSampleRate;
        }

        int GetOutputSampleRate() {
            return _outputSampleRate;
        }

        void SetInputSampleRate(int sampleRate) {
            _inputSampleRate = sampleRate;
        }

        void SetOutputSampleRate(int sampleRate) {
            _outputSampleRate = sampleRate;
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

        int GetRtlsdrCorrection() {
            return _rtlsdrCorrection;
        }

        int GetRtlsdrOffset() {
            return _rtlsdrOffset;
        }

        int GetRtlsdrAdjust() {
            return _rtlsdrAdjust;
        }

        int GetShift() {
            return _shift;
        }

        std::string GetDumpFileSuffix() {
            return _dumpFileSuffix;
        }

        std::string GetOutputFilename() {
            return _outputFilename;
        }

        int GetDecimatorGain() {
            return _decimatorGain;
        }

        int GetDecimatorCutoff() {
            return _rtlsdrOffset * 2;
        }

        int GetRtlsdrCorrectionFactor() {
            return _rtlsdrCorrectionFactor;
        }

        int GetFirFilterSize() {
            return _firFilterSize;
        }

        int GetDecimatorAgcLevel() {
            return _decimatorAgcLevel;
        }

        bool GetFrequencyAlign() {
            return _frequencyAlign;
        }

        int GetFrequencyAlignVolume() {
            return _frequencyAlignVolume;
        }

        std::map<int, Channel*> GetChannels() {
            std::map<int, Channel*> channels;
            int number = 1;
            for( std::vector<Channel*>::iterator it = _channels.begin(); it != _channels.end(); it++ ) {
                channels.insert(std::pair<int, Channel*>(number, *it));
            }
            return channels;
        }

        bool AddChannel(std::string name, long int frequency) {
            _channels.push_back(new Channel(name, frequency));
            std::sort(_channels.begin(), _channels.end(), Channel::comparator);
            return true;
        }

        bool RemoveChannel(int id) {
            std::map<int, Channel*> channels = GetChannels();
            if( id > 0 && id <= channels.size() ) {
                std::vector<Channel*>::iterator it = _channels.begin();
                int number = 0;
                while( it != _channels.end() ) {
                    if( number + 1 == id ) {
                        _channels.erase(it);
                        return true;
                    }
                }
            }
            return false;
        }

        void SetReceiverOptionsFor(std::string receiver, std::map<std::string, std::string> options);

        std::map<std::string, std::string> GetReceiverOptionsFor(std::string receiver);

        void WriteBookmark(std::string name);

        bool ReadBookmark(std::string name);

        void DeleteBookmark(std::string name);

        std::vector<std::string> ListBookmarks();
    };

#endif
