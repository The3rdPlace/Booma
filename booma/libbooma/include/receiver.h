#ifndef __RECEIVER_H
#define __RECEIVER_H

#include <hardtapi.h>
#include "config.h"
#include "input.h"

class BoomaReceiver {

    private:

        int _sampleRate;
        HWriterConsumer<int16_t>* _preProcess;
        HWriterConsumer<int16_t>* _receive;
        HWriterConsumer<int16_t>* _postProcess;
        HSplitter<int16_t>* _spectrum;

    protected:

        int GetSampleRate() {
            return _sampleRate;
        }

        virtual HWriterConsumer<int16_t>* PreProcess(ConfigOptions* opts, HWriterConsumer<int16_t>* previous) = 0;
        virtual HWriterConsumer<int16_t>* Receive(ConfigOptions* opts, HWriterConsumer<int16_t>* previous) = 0;
        virtual HWriterConsumer<int16_t>* PostProcess(ConfigOptions* opts, HWriterConsumer<int16_t>* previous) = 0;

    public:

        BoomaReceiver(ConfigOptions* opts):
            _sampleRate(opts->GetSampleRate()) {}

        virtual ~BoomaReceiver() {
            delete _spectrum;
        }

        void Build(ConfigOptions* opts, BoomaInput* input) {

            // Add preprocessing part of the receiver
            _preProcess = PreProcess(opts, input->GetLastWriterConsumer());

            // Add a splitter so that we can take the full spectrum out before running through the receiver filters
            _spectrum = new HSplitter<int16_t>(_preProcess);

            // Add the receiver chain
            _receive = Receive(opts, _spectrum->Consumer());

            // Add postprocessing part of the receiver
            _postProcess = PostProcess(opts, _receive);
        };

        virtual bool SetFrequency(long int frequency) = 0;
        virtual bool SetRfGain(int gain) = 0;

        HWriterConsumer<int16_t>* GetLastWriterConsumer() {
            return _postProcess;
        }

        HWriterConsumer<int16_t>* GetSpectrumConsumer() {
            return _spectrum->Consumer();
        }
};

#endif
