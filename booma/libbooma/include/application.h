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
            current = new std::thread( [this]()  { this->processor->Run(); } );
        }

        void Halt() {
            IsTerminated = true;
            if( current != NULL ) {
                current->join();
                current = NULL;
            }
        }

        bool SetReceiver();

        HProcessor<int16_t>* processor;

        bool SetFrequency(long int frequency);
        bool ChangeFrequency(int stepSize);
        bool ChangeVolume(int stepSize);

    private:

        ConfigOptions* _opts;
        bool IsTerminated;
        std::thread* current;

        HReader<int16_t>* inputReader;
        HGain<int16_t>* outputWriter;
        HSoundcardWriter<int16_t>* _soundcardWriter;

        HWriter<int16_t>* pcmWriter;
        HWriter<int16_t>* audioWriter;
        HSplitter<int16_t>* pcmSplitter;
        HSplitter<int16_t>* audioSplitter;
        HMute<int16_t>* pcmMute;
        HMute<int16_t>* audioMute;

        BoomaReceiver* receiver;

        bool SetInput();
        bool SetOutput();
        bool SetDumps();

};

#endif