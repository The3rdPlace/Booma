#ifndef __APPLICATION_H
#define __APPLICATION_H

#include <thread>

#include <hardtapi.h>

#include "config.h"
#include "input.h"
#include "receiver.h"
#include "output.h"
#include "booma.h"

class BoomaApplication {

    public:

        // constructor and destructor
        BoomaApplication(std::string appName, std::string appVersion, int argc, char** argv);
        ~BoomaApplication();

        // Run receiver chain
        void Run() {
	    HLog("Run receiver chain");
            _isTerminated = false;
            _current = new std::thread( [this]()  {
                _input->Run( _opts->GetEnableProbes() ? 100 : 0);
                _isTerminated = true;
                _isRunning = false;
                _current = NULL;
            } );
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
        bool SetFrequency(long int frequency);
        long int GetFrequency();
        bool ChangeFrequency(int stepSize);
        bool SetVolume(int volume);
        bool ChangeVolume(int stepSize);
        int GetVolume();
        bool ToggleDumpRf();
        bool GetDumpRf();
        bool ToggleDumpAudio();
        bool GetDumpAudio();
        bool SetRfGain(int gain);
        bool ChangeRfGain(int stepSize);
        int GetRfGain();
        int GetSampleRate() {
            return SAMPLERATE;
        }

        // Public reporting functions
        int GetSignalLevel();
        int GetRfFftSize();
        int GetRfSpectrum(double* spectrum);
        int GetAudioFftSize();
        int GetAudioSpectrum(double* spectrum);

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

        // Convenience functions for creating a receiver chain
        bool InitializeReceiver();
};

#endif
