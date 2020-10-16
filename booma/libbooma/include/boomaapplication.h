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
                HLog("Unable to run, receiver is NULL");
                return;
            }
	        HLog("Run receiver chain");
            _isTerminated = false;
            if( _opts->GetEnableProbes() ) {
                HLog("Probes enabled, running 200 blocks");
                _input->Run( 200 );
                _isTerminated = true;
                _isRunning = false;
                _current = NULL;
            } else {
                HLog("Starting normal run");
                _current = new std::thread( [this]()  {
                    _input->Run();
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

        // Get the IF frequency, the frequency where the received signal is
        // located in the frequency band provided to the receiver
        int GetIfFrequency();

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

        int GetSampleRate() {
            return SAMPLERATE;
        }

        bool GetEnableProbes() {
            return _opts->GetEnableProbes();
        }

        // Public control functions that would require a receiver restart after modifications
        InputSourceType GetInputSourceType();
        InputSourceDataType GetInputSourceDataType();
        int GetInputDevice();
        std::string GetPcmFile();
        std::string GetWavFile();
        int GetSignalGeneratorFrequency();
        std::string GetRemoteServer();
        int GetRemoteDataPort();
        int GetRemoteCommandPort();

        // Public reporting functions for spectrum and signallevel
        int GetSignalLevel();
        double GetSignalSum();
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

        // Bookmarks
        void SetBookmark(std::string name);
        std::vector<std::string> GetBookmarks();
        void ApplyBookmark(std::string name);
        void DeleteBookmark(std::string name);

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
