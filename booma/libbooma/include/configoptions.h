#ifndef __CONFIGOPTIONS_H
#define __CONFIGOPTIONS_H

#include <stdlib.h>
#include <iostream>
#include <cstring>

#include <hardtapi.h>
#include "channel.h"
#include "configoptionvalues.h"

#define CONFIGNAME "config.ini"

/** Configuration object */
class ConfigOptions {

    private:

        // Values
        std::map<std::string, ConfigOptionValues*> _values;

        // Memory channels
        std::vector<Channel*> _channels;

        // Current config section
        std::string _section = "default";

        void PrintUsage(bool showSecretSettings = false);
        void PrintAudioDevices();
        void PrintRtlsdrDevices();
        std::string GetAudioDevice(int device);
        std::string GetRtlsdrDevice(int device);

        bool ReadStoredConfig(std::string configname, bool isBookmark);
        void WriteStoredConfig(std::string configname, bool isBookmark);
        void RemoveStoredConfig(std::string configname);
        std::map<std::string, std::string> ReadStoredReceiverOptions(std::string optionsString);
        std::string WriteStoredReceiverOptions(std::map<std::string, std::string> options);
        std::map<std::string, std::map<std::string, std::string>> ReadStoredReceiverOptionsFor(std::string optionsForString);
        std::string WriteStoredReceiverOptionsFor(std::map<std::string,std::map<std::string, std::string>> options);
        std::vector<Channel*> ReadChannels(std::string configname, std::string channels);
        std::vector<Channel*> ReadPersistentChannels(std::string configname);
        std::string WriteChannels(std::string configname, std::vector<Channel*> channels);
        void DumpConfigInfo();

    public:

        ConfigOptions(std::string appName, std::string appVersion, int argc, char** argv);

        ~ConfigOptions();

        static bool IsVerbose(int argc, char** argv);

        int GetOutputAudioDevice() {
            return _values.at(_section)->_outputAudioDevice;
        }

        InputSourceType GetInputSourceType() {
            return _values.at(_section)->_inputSourceType;
        }

        InputSourceType GetOriginalInputSourceType() {
            return _values.at(_section)->_originalInputSourceType == NO_INPUT_SOURCE_TYPE
            ? _values.at(_section)->_inputSourceType
            : _values.at(_section)->_originalInputSourceType;
        }

        InputSourceDataType GetInputSourceDataType() {
            return _values.at(_section)->_inputSourceDataType;
        }

        int GetInputDevice() {
            return _values.at(_section)->_inputDevice;
        }

        long int GetFrequency() {
            return _values.at(_section)->_frequency;
        }

        void SetFrequency(long int frequency) {
            _values.at(_section)->_frequency = frequency;
        }

        ReceiverModeType GetReceiverModeType() {
            return _values.at(_section)->_receiverModeType;
        }

        void SetReceiverModeType(ReceiverModeType receiverModeType) {
            _values.at(_section)->_receiverModeType = receiverModeType;
        }

        bool GetIsRemoteHead() {
            return _values.at(_section)->_isRemoteHead;
        }

        std::string GetRemoteServer() {
            return _values.at(_section)->_remoteServer;
        }

        int GetRemoteDataPort() {
            return _values.at(_section)->_remoteDataPort;
        }

        int GetRemoteCommandPort() {
            return _values.at(_section)->_remoteCommandPort;
        }

        bool GetUseRemoteHead() {
            return _values.at(_section)->_useRemoteHead;
        }

        int GetRfGain() {
            return _values.at(_section)->_rfGain;
        }

        void SetRfGain(int gain) {
            _values.at(_section)->_rfGain = gain;
        }

        int GetRfAgcLevel() {
            return _values.at(_section)->_rfAgcLevel;
        }

        int GetVolume() {
            return _values.at(_section)->_volume;
        }

        void SetVolume(int volume) {
            _values.at(_section)->_volume = volume;
        }

        bool GetDumpRf() {
            return _values.at(_section)->_dumpRf;
        }

        void SetDumpRf(bool enabled) {
            _values.at(_section)->_dumpRf = enabled;
        }

        bool GetDumpAudio() {
            return _values.at(_section)->_dumpAudio;
        }

        void SetDumpAudio(bool enabled) {
            _values.at(_section)->_dumpAudio = enabled;
        }

        DumpFileFormatType GetDumpRfFileFormat() {
            return _values.at(_section)->_dumpRfFileFormat;
        }

        DumpFileFormatType GetDumpAudioFileFormat() {
            return _values.at(_section)->_dumpAudioFileFormat;
        }

        int GetSignalGeneratorFrequency() {
            return _values.at(_section)->_signalGeneratorFrequency;
        }

        int GetInputSampleRate() {
            return _values.at(_section)->_inputSampleRate;
        }

        int GetOutputSampleRate() {
            return _values.at(_section)->_outputSampleRate;
        }

        void SetInputSampleRate(int sampleRate) {
            _values.at(_section)->_inputSampleRate = sampleRate;
        }

        void SetOutputSampleRate(int sampleRate) {
            _values.at(_section)->_outputSampleRate = sampleRate;
        }

        std::string GetPcmFile() {
            return _values.at(_section)->_pcmFile;
        }

        std::string GetWavFile() {
            return _values.at(_section)->_wavFile;
        }

        bool GetEnableProbes() {
            return _values.at(_section)->_enableProbes;
        }

        int GetReservedBuffers() {
            return _values.at(_section)->_reservedBuffers;
        }

        bool GetEnableBuffers() {
            return _values.at(_section)->_reservedBuffers > 0;
        }

        HTimer GetSchedule() {
            return _values.at(_section)->_schedule;
        }

        std::map<std::string, std::string>* GetReceiverOptions() {
            return &_values.at(_section)->_receiverOptions;
        }

        int GetRtlsdrCorrection() {
            return _values.at(_section)->_rtlsdrCorrection;
        }

        int GetRtlsdrOffset() {
            return _values.at(_section)->_rtlsdrOffset;
        }

        int GetRtlsdrAdjust() {
            return _values.at(_section)->_rtlsdrAdjust;
        }

        int GetShift() {
            return _values.at(_section)->_shift;
        }

        std::string GetDumpFileSuffix() {
            return _values.at(_section)->_dumpFileSuffix;
        }

        std::string GetOutputFilename() {
            return _values.at(_section)->_outputFilename;
        }

        int GetDecimatorGain() {
            return _values.at(_section)->_decimatorGain;
        }

        int GetDecimatorCutoff() {
            return _values.at(_section)->_rtlsdrOffset * 2;
        }

        int GetRtlsdrCorrectionFactor() {
            return _values.at(_section)->_rtlsdrCorrectionFactor;
        }

        int GetRtlsdrGain() {
            return _values.at(_section)->_rtlsdrGain;
        }

        int GetFirFilterSize() {
            return _values.at(_section)->_firFilterSize;
        }

        int GetDecimatorAgcLevel() {
            return _values.at(_section)->_decimatorAgcLevel;
        }

        bool GetFrequencyAlign() {
            return _values.at(_section)->_frequencyAlign;
        }

        int GetFrequencyAlignVolume() {
            return _values.at(_section)->_frequencyAlignVolume;
        }

        std::map<int, Channel*> GetChannels() {
            std::map<int, Channel*> channels;
            int number = 1;
            for( std::vector<Channel*>::iterator it = _channels.begin(); it != _channels.end(); it++ ) {
                channels.insert(std::pair<int, Channel*>(number++, *it));
            }
            return channels;
        }

        bool AddChannel(std::string name, long int frequency) {
            _channels.push_back(new Channel(name, frequency));
            std::sort(_channels.begin(), _channels.end(), Channel::ChannelComparator());
            return true;
        }

        bool RemoveChannel(int id) {
            std::map<int, Channel*> channels = GetChannels();
            if( id > 0 && id <= channels.size() ) {
                std::vector<Channel*>::iterator it = _channels.begin();
                int number = 0;
                while( it != _channels.end() ) {
                    std::cout << "NAME " << (*it)->Name << " with number " << (number + 1) << " <== " << id << std::endl;
                    if( number + 1 == id ) {
                        _channels.erase(it);
                        return true;
                    }
                    number++;
                    it++;
                }
            }
            return false;
        }

        bool SetInputFilterWidth(int width) {
            _values.at(_section)->_inputFilterWidth = width;
            return true;
        }

        int GetInputFilterWidth() {
            return _values.at(_section)->_inputFilterWidth;
        }

        void SetReceiverOptionsFor(std::string receiver, std::map<std::string, std::string> options);

        std::map<std::string, std::string> GetReceiverOptionsFor(std::string receiver);

        void WriteBookmark(std::string name);

        bool ReadBookmark(std::string name);

        void DeleteBookmark(std::string name);

        std::vector<std::string> ListBookmarks();
    };

#endif
