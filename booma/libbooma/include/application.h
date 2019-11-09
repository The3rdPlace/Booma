#ifndef __APPLICATION_H
#define __APPLICATION_H

#include <thread>

#include <hardtapi.h>

#include "config.h"
#include "receiver.h"

class BoomaApplication {

    public:


        BoomaApplication(std::string appName, std::string appVersion, int argc, char** argv, bool verbose);

        ~BoomaApplication();

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

        void Halt() {
            IsTerminated = true;
            if( _current != NULL ) {
                _current->join();
                _current = NULL;
            }
            IsTerminated = false;
            _isRunning = false;
        }

        bool ChangeReceiver();
        bool ChangeReceiver(ReceiverModeType receiverModeType);

        HProcessor<int16_t>* processor;

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

    private:

        ConfigOptions* _opts;
        bool IsTerminated;
        bool _isRunning;
        std::thread* _current;

        HReader<int16_t>* _inputReader;
        HGain<int16_t>* _outputWriter;
        HSoundcardWriter<int16_t>* _soundcardWriter;
        HNullWriter<int16_t>* _nullWriter;

        HWriter<int16_t>* _rfWriter;
        HWriter<int16_t>* _audioWriter;
        HSplitter<int16_t>* _rfSplitter;
        HSplitter<int16_t>* _audioSplitter;
        HMute<int16_t>* _rfMute;
        HMute<int16_t>* _audioMute;

        HSignalLevel<int16_t>* _signalLevel;
        HCustomWriter<HSignalLevelResult>* _signalLevelWriter;
        int SignalLevelCallback(HSignalLevelResult* result, size_t length);
        int _signalLevels[16];
        bool _firstLevel;

        BoomaReceiver* _receiver;

        bool SetInput();
        bool SetReceiver();
        bool SetOutput();

        bool InitializeReceiver();
};

#endif