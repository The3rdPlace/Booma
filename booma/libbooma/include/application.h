#ifndef __APPLICATION_H
#define __APPLICATION_H

#include <thread>

#include <hardtapi.h>

#include "config.h"
#include "receiver.h"
#include "booma.h"

class BoomaApplication {

    public:


        // constructor and destructor
        BoomaApplication(std::string appName, std::string appVersion, int argc, char** argv, bool verbose);
        ~BoomaApplication();

        // Run receiver chain
        void Run() {
            IsTerminated = false;
            _current = new std::thread( [this]()  {
                this->processor->Run();
                IsTerminated = true;
                _isRunning = false;
                _current = NULL;
            } );
            _isRunning = true;
        }

        // Halt receiver chain
        void Halt() {
            IsTerminated = true;
            if( _current != NULL ) {
                _current->join();
                _current = NULL;
            }
            IsTerminated = false;
            _isRunning = false;
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
        int GetSignalLevel();
        int GetRfFftSize();
        int GetRfSpectrum(double* spectrum);
        int GetSampleRate() {
            return SAMPLERATE;
        }

    private:

        // Configuration and state
        ConfigOptions* _opts;
        bool IsTerminated;
        bool _isRunning;
        std::thread* _current;

        HProcessor<int16_t>* processor;

        // In- and output
        HReader<int16_t>* _inputReader;
        HGain<int16_t>* _outputWriter;
        HSoundcardWriter<int16_t>* _soundcardWriter;
        HNullWriter<int16_t>* _nullWriter;

        // Splitting audio and RF
        HWriter<int16_t>* _rfWriter;
        HWriter<int16_t>* _audioWriter;
        HSplitter<int16_t>* _rfSplitter;
        HSplitter<int16_t>* _audioSplitter;
        HMute<int16_t>* _rfMute;
        HMute<int16_t>* _audioMute;

        // Signal level reporting
        HSignalLevel<int16_t>* _signalLevel;
        HCustomWriter<HSignalLevelResult>* _signalLevelWriter;
        int SignalLevelCallback(HSignalLevelResult* result, size_t length);
        int _signalLevels[SIGNALLEVEL_AVERAGING_COUNT];
        bool _firstSignalLevel;

        // RF spectrum reporting
        HFft<int16_t>* _rfFft;
        HCustomWriter<HFftResults>* _rfFftWriter;
        int RfFftCallback(HFftResults* result, size_t length);
        HRectangularWindow<int16_t>* _rfFftWindow;
        double* _rfSpectrum;
        int _rfFftSize;
        bool _firstRfSpectrum;

        // The active receiver
        BoomaReceiver* _receiver;

        // Convenience functions for creating a receiver chain
        bool SetInput();
        bool SetReceiver();
        bool SetOutput();
        bool InitializeReceiver();
};

#endif