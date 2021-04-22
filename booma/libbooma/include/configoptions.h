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

        // Current config section
        std::string _section = "default";
        std::string _activeSection = "default";

        void PrintUsage(bool showSecretSettings = false);
        void PrintAudioDevices(bool hardwareDevices = true, bool virtualDevices = false);
        void PrintRtlsdrDevices();

        bool ReadStoredConfig(std::string configname, bool isBookmark);
        void WriteStoredConfig(std::string configname, bool isBookmark);
        void RemoveStoredConfig(std::string configname);
        std::map<std::string, std::string> ReadStoredReceiverOptions(std::string optionsString);
        std::string WriteStoredReceiverOptions(std::map<std::string, std::string> options);
        std::map<std::string, std::map<std::string, std::string>> ReadStoredReceiverOptionsFor(std::string optionsForString);
        std::string WriteStoredReceiverOptionsFor(std::map<std::string,std::map<std::string, std::string>> options);
        std::vector<Channel*> ReadChannels(std::string configname, std::string channels);
        std::vector<Channel*> ReadPersistentChannels(std::string configname, std::string section);
        std::string WriteChannels(std::string configname, std::string section, std::vector<Channel*> channels);
        void DumpConfigInfo();

    public:

        ConfigOptions(std::string appName, std::string appVersion, int argc, char** argv);
        ~ConfigOptions();

        void SyncStoredConfig();

        std::string GetAudioDevice(int device);
        std::string GetRtlsdrDevice(int device);
        std::map<int, std::string> GetAudioDevices(bool hardwareDevices = true, bool virtualDevices = false, bool inputs = true, bool outputs = true);
        std::map<int, std::string> GetRtlsdrDevices();
        static bool IsVerbose(int argc, char** argv);

        int GetOutputAudioDevice() {
            return _values.at(_section)->_outputAudioDevice;
        }

        InputSourceType GetInputSourceType() {
            return _values.at(_section)->_inputSourceType;
        }

        bool SetInputSourceType(InputSourceType inputSourceType) {
            _values.at(_section)->_inputSourceType = inputSourceType;
            _values.at(_section)->_isRemoteHead = (inputSourceType == NETWORK);
            return true;
        }

        InputSourceType GetOriginalInputSourceType() {
            return _values.at(_section)->_originalInputSourceType == NO_INPUT_SOURCE_TYPE
            ? _values.at(_section)->_inputSourceType
            : _values.at(_section)->_originalInputSourceType;
        }

        bool SetOriginalInputSourceType(InputSourceType originalInputSourceType) {
            _values.at(_section)->_originalInputSourceType = originalInputSourceType;
            return true;
        }

        InputSourceDataType GetInputSourceDataType() {
            return _values.at(_section)->_inputSourceDataType;
        }

        bool SetInputSourceDataType(InputSourceDataType inputSourceDataType) {
            _values.at(_section)->_inputSourceDataType = inputSourceDataType;
            return true;
        }

        int GetInputDevice() {
            return _values.at(_section)->_inputDevice;
        }

        bool SetInputDevice(int device) {
            _values.at(_section)->_inputDevice = device;
            return true;
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

        bool SetRemoteServer(std::string server) {
            _values.at(_section)->_remoteServer = server;
            return true;
        }

        int GetRemoteDataPort() {
            return _values.at(_section)->_remoteDataPort;
        }

        bool SetRemoteDataPort(int portnumber) {
            _values.at(_section)->_remoteDataPort = portnumber;
            return true;
        }

        int GetRemoteCommandPort() {
            return _values.at(_section)->_remoteCommandPort;
        }

        bool SetRemoteCommandPort(int portnumber) {
            _values.at(_section)->_remoteCommandPort = portnumber;
            return true;
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

        bool SetSignalGeneratorFrequency(int frequency) {
            _values.at(_section)->_signalGeneratorFrequency = frequency;
            return true;
        }

        int GetInputSampleRate() {
            return _values.at(_section)->_inputSampleRate;
        }

        int GetOutputSampleRate() {
            return _values.at(_section)->_outputSampleRate;
        }

        bool SetInputSampleRate(int sampleRate) {
            _values.at(_section)->_inputSampleRate = sampleRate;
            return true;
        }

        bool SetOutputSampleRate(int sampleRate) {
            _values.at(_section)->_outputSampleRate = sampleRate;
            return true;
        }

        std::string GetPcmFile() {
            return _values.at(_section)->_pcmFile;
        }

        bool SetPcmFile(std::string filename) {
            _values.at(_section)->_pcmFile = filename;
            return true;
        }

        std::string GetWavFile() {
            return _values.at(_section)->_wavFile;
        }

        bool SetWavFile(std::string filename) {
            _values.at(_section)->_wavFile = filename;
            return true;
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

        long GetRtlsdrAdjust() {
            return _values.at(_section)->_rtlsdrAdjust;
        }

        bool SetRtlsdrAdjust(long adjust) {
            _values.at(_section)->_rtlsdrAdjust = adjust;
            return true;
        }

        long GetShift() {
            return _values.at(_section)->_shift;
        }

        bool SetShift(long shift) {
            _values.at(_section)->_shift = shift;
            return true;
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
            for( std::vector<Channel*>::iterator it = _values.at(_section)->_channels.begin(); it != _values.at(_section)->_channels.end(); it++ ) {
                channels.insert(std::pair<int, Channel*>(number++, *it));
            }
            return channels;
        }

        bool AddChannel(std::string name, long int frequency) {
            _values.at(_section)->_channels.push_back(new Channel(name, frequency));
            std::sort(_values.at(_section)->_channels.begin(), _values.at(_section)->_channels.end(), Channel::ChannelComparator());
            return true;
        }

        bool RemoveChannel(int id) {
            std::map<int, Channel*> channels = GetChannels();
            if( id > 0 && id <= channels.size() ) {
                std::vector<Channel*>::iterator it = _values.at(_section)->_channels.begin();
                int number = 0;
                while( it != _values.at(_section)->_channels.end() ) {
                    std::cout << "NAME " << (*it)->Name << " with number " << (number + 1) << " <== " << id << std::endl;
                    if( number + 1 == id ) {
                        _values.at(_section)->_channels.erase(it);
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

        std::vector<std::string> GetConfigSections() {
            std::vector<std::string> sections;
            for( std::map<std::string, ConfigOptionValues*>::iterator it = _values.begin(); it != _values.end(); it++ ) {
                sections.push_back((*it).first);
            }
            return sections;
        }

        std::string GetConfigSection() {
            return _section;
        }

        bool SetConfigSection(std::string section) {
            if( _values.find(section) == _values.end() ) {
                HLog("Config section %s does not exist", section.c_str());
                return false;
            }
            HLog("Set section %s as active", _section.c_str());
            _section = section;
            return true;
        }

        bool CreateConfigSection(std::string section, bool cloneOldSettings, bool replaceDefault) {
            if( _values.find(section) != _values.end() ) {
                HLog("Config section %s exists", section.c_str());
                return false;
            }

            // Clone current settings
            if( cloneOldSettings ) {
                ConfigOptionValues* newValues = new ConfigOptionValues(_values.at(_section));
                _values.insert(std::pair<std::string, ConfigOptionValues*>(section, newValues));
            } else {
                ConfigOptionValues* newValues = new ConfigOptionValues();
                _values.insert(std::pair<std::string, ConfigOptionValues*>(section, newValues));
            }

            // Replace current 'default' section ?
            if( _section == "default" && replaceDefault ) {
                std::map<std::string, ConfigOptionValues*>::iterator old = _values.find(_section);
                _values.erase(old);
                HLog("Erase old 'default' config section");
            }

            HLog("Set section %s as active", _section.c_str()   );
            _section = section;
            return true;
        }

        bool RenameConfigSection(std::string newname) {
            if( _values.find(newname) != _values.end() ) {
                HLog("Config section %s exists", newname.c_str());
                return false;
            }

            // Clone current settings
            ConfigOptionValues* newValues = new ConfigOptionValues(_values.at(_section));
            _values.insert(std::pair<std::string, ConfigOptionValues*>(newname, newValues));
            std::map<std::string, ConfigOptionValues*>::iterator old = _values.find(_section);
            _values.erase(old);
            HLog("Renamed section '%s' to '%s'", _section.c_str(), newname.c_str());
            _section = newname;
            return true;
        }

        bool DeleteConfigSection(std::string section) {
            if( section == _section ) {
                HLog("Unable to delete active config section %s", section.c_str());
                return false;
            }

            std::map<std::string, ConfigOptionValues*>::iterator it = _values.find(section);
            if( it == _values.end() ) {
                HLog("Config section %s does not exist", section.c_str());
                return false;
            }

            _values.erase(it);
            return true;
        }

        void SetOutputAudioDevice(int card) {
            _values.at(_section)->_outputAudioDevice = card;
        }

        void SetOutputFilename(std::string filename) {
            _values.at(_section)->_outputFilename = filename;
        }

        void SetPreamp(int preamp) {
            _values.at(_section)->_preamp = preamp;
        }

        int GetPreamp() {
            return _values.at(_section)->_preamp;
        }

        void SetRfGainEnabled(bool enabled) {
            _values.at(_section)->_rfGainEnabled = enabled;
        }

        bool GetRfGainEnabled() {
            return _values.at(_section)->_rfGainEnabled;
        }
    };

#endif
