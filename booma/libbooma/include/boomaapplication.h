#ifndef __APPLICATION_H
#define __APPLICATION_H

#include <thread>

#include <hardtapi.h>

#include "configoptions.h"
#include "boomainput.h"
#include "boomareceiver.h"
#include "boomaoutput.h"
#include "booma.h"
#include "option.h"

class BoomaApplication {

    public:

        // constructor and destructor
        BoomaApplication(std::string appName, std::string appVersion, int argc, char** argv);
        ~BoomaApplication();

        // Run receiver chain
        void Run() {
            if( _input == NULL ) {
                HLog("Unable to run, input is NULL");
                return;
            }
	        HLog("Run receiver chain");
            _isTerminated = false;
            if( _opts->GetEnableProbes() ) {
                HLog("Probes enabled, running 200 blocks");
                _input->Run( 200 );

                HLog("Resetting running state");
                _isTerminated = true;
                _isRunning = false;
                _current = NULL;
            } else {
                HLog("Starting normal run");
                _current = new std::thread( [this]()  {
                    try {
                        _input->Run();
                    } catch( BoomaConfigurationException e ) {
                        HError("Caught exception from Run() of type '%s' with error '%s'", e.Type().c_str(), e.What().c_str());
                    } catch( BoomaInputException e ) {
                        HError("Caught exception from Run() of type '%s' with error '%s'", e.Type().c_str(), e.What().c_str());
                    } catch( BoomaReceiverException e ) {
                        HError("Caught exception from Run() of type '%s' with error '%s'", e.Type().c_str(), e.What().c_str());
                    } catch( HException e ) {
                        HError("Caught exception from Run() of type '%s' with error '%s'", e.type(), e.what());
                    } catch( ... ) {
                        HError("Caught unknown exception from Run()");
                    }

                    HLog("Resetting running state");
                    _isTerminated = true;
                    _isRunning = false;
                    _current = NULL;
                } );
            }
            _isRunning = true;
            HLog("Receiver chain is running");
        }

        // Halt receiver chain
        void Halt(bool wait = true) {
	        HLog("Halt receiver chain");
	        if( !_isRunning ) {
	            HLog("Already halted");
	            return;
	        }
            _isTerminated = true;
            if( wait )
            {
	            HLog("Wait for receiver chain to halt");
                if( _current != NULL ) {
 		            HLog("Joining active thread");
                    _current->join();
                    _current = NULL;
                    HLog("Thread joined");
                }

		        HLog("Resetting running state");
                _isTerminated = false;
                _isRunning = false;
            }
            HLog("Receiver chain is halted");
        }

        // Wait for the receiver chain to exit
        void Wait() {
            HLog("Waiting for active receiver chain to halt");
            if( _current != NULL ) {
	            HLog("Has active thread, joining");
                _current->join();
                _current = NULL;
                HLog("Active thread halted");
            }
            _isRunning = false;
            HLog("Active receiver chain has halted");
        }

        // Public control functions
        bool ChangeReceiver();
        bool ChangeReceiver(ReceiverModeType receiverModeType);
        bool InitializeReceiver();
        ReceiverModeType GetReceiver();

        // Set and get the virtual frequency - the frequency the user inputs
        // and what is show on a scale dial or in a display.
        // This is the frequency in the spectrum that is actually being received
        // but it may appear as a virtual frequency due to use of devices that translates
        // this frequency by use of mixers into a lower frequency band.
        long int GetFrequency();
        bool SetFrequency(long int frequency);
        bool ChangeFrequency(int stepSize);

        // Get frequency shift and frequency adjustments for rtl-sdr dongles
        long GetShift();
        bool SetShift(long shift);
        long GetRealShift();
        long GetFrequencyAdjust();
        long GetRealFrequencyAdjust();
        bool SetFrequencyAdjust(long adjust);

        // Set 1.st IF filter width
        bool SetInputFilterWidth(int width);
        int GetInputFilterWidth();

        int GetVolume();
        bool SetVolume(int volume);
        bool ChangeVolume(int stepSize);

        bool GetDumpRf();
        bool ToggleDumpRf();

        bool GetDumpAudio();
        bool ToggleDumpAudio();

        int GetRfGain();
        bool SetRfGain(int gain);
        bool ChangeRfGain(int stepSize);

        int GetDefaultSampleRate() {
            return SAMPLERATE;
        }

        bool GetEnableProbes() {
            return _opts->GetEnableProbes();
        }

        // Public control functions that would require a receiver restart after modifications
        InputSourceType GetInputSourceType();
        bool SetInputSourceType(InputSourceType inputSourceType);
        InputSourceType GetOriginalInputSourceType();
        bool SetOriginalInputSourceType(InputSourceType originalInputSourceType);
        InputSourceDataType GetInputSourceDataType();
        bool SetInputSourceDataType(InputSourceDataType inputSourceDataType);
        int GetInputDevice();
        bool SetInputDevice(int device);
        std::string GetPcmFile();
        bool SetPcmFile(std::string filename);
        std::string GetWavFile();
        bool SetWavFile(std::string filename);
        int GetSignalGeneratorFrequency();
        bool SetSignalGeneratorFrequency(int frequency);
        std::string GetRemoteServer();
        bool SetRemoteServer(std::string server);
        int GetRemoteDataPort();
        bool SetRemoteDataPort(int portnumber);
        int GetRemoteCommandPort();
        bool SetRemoteCommandPort(int portnumber);
        std::string GetOutputFilename();
        int GetOutputDevice();
        void SetOutputAudioDevice(int card);
        void SetOutputFilename(std::string filename);

        // Public reporting functions for spectrum and signallevel
        int GetSignalLevel();
        double GetSignalSum();
        int GetSignalMax();
        int GetRfFftSize();
        int GetRfSpectrum(double* spectrum);
        int GetAudioFftSize();
        int GetAudioSpectrum(double* spectrum);

        // Schedule
        HTimer GetSchedule();

        // Other reporting functions
        bool GetEnableBuffers();
        std::vector<Option>* GetOptions();
        int GetOption(std::string name);
        bool SetOption(std::string name, std::string value);
        std::string GetOptionInfoString();
        int GetOutputFilterWidth();
        std::map<int, std::string> GetAudioDevices(bool hardwareDevices, bool virtualDevices, bool inputs, bool outputs);
        std::map<int, std::string> GetRtlsdrDevices();
        int GetInputSampleRate();
        bool SetInputSampleRate(int rate);
        int GetOutputSampleRate();
        bool SetOutputSampleRate(int rate);

        // Bookmarks
        void SetBookmark(std::string name);
        std::vector<std::string> GetBookmarks();
        void ApplyBookmark(std::string name);
        void DeleteBookmark(std::string name);

        // Channels
        std::map<int, Channel*> GetChannels();
        bool AddChannel(std::string name, long int frequency);
        bool RemoveChannel(int id);
        bool UseChannel(int id);

        // Config sections
        std::vector<std::string> GetConfigSections();
        std::string GetConfigSection();
        bool SetConfigSection(std::string section);
        bool CreateConfigSection(std::string section, bool cloneOldSettings = true, bool replaceDefault = true);
        bool DeleteConfigSection(std::string section);
        bool RenameConfigSection(std::string newname);

        // Configuration
        void SyncConfiguration();

    private:

        // Configuration and state
        ConfigOptions* _opts;
        bool _isTerminated;
        bool _isRunning;
        std::thread* _current;

        // Structural blocks
        BoomaInput* _input;
        BoomaReceiver* _receiver;
        BoomaOutput* _output;

        // Disable copy constructor usage since that would
        // create multiple instances of the application core!
        BoomaApplication(const BoomaApplication&);
        BoomaApplication& operator=(const BoomaApplication&);

        // Reconfigure the entire receiver
        bool Reconfigure();
};

#endif
