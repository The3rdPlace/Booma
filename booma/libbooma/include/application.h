#ifndef __APPLICATION_H
#define __APPLICATION_H

#include <thread>

#include <hardtapi.h>

#include "config.h"
#include "receiver.h"

class BoomaApplication {

    public:


        BoomaApplication(int argc, char** argv, bool verbose);

        void Run() {
            IsTerminated = false;
            _current = new std::thread( [this]()  { this->processor->Run(); } );
        }

        void Halt() {
            IsTerminated = true;
            if( _current != NULL ) {
                _current->join();
                _current = NULL;
            }
        }

        bool SetReceiver();

        HProcessor<int16_t>* processor;

        bool SetFrequency(long int frequency);
        bool ChangeFrequency(int stepSize);
        bool ChangeVolume(int stepSize);
        bool ToggleDumpPcm();
        bool ToggleDumpAudio();

    private:

        ConfigOptions* _opts;
        bool IsTerminated;
        std::thread* _current;

        HReader<int16_t>* _inputReader;
        HGain<int16_t>* _outputWriter;
        HSoundcardWriter<int16_t>* _soundcardWriter;

        HWriter<int16_t>* _pcmWriter;
        HWriter<int16_t>* _audioWriter;
        HSplitter<int16_t>* _pcmSplitter;
        HSplitter<int16_t>* _audioSplitter;
        HMute<int16_t>* _pcmMute;
        HMute<int16_t>* _audioMute;

        BoomaReceiver* _receiver;

        bool SetInput();
        bool SetOutput();
        bool SetDumps();

};

#endif