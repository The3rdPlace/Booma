#ifndef __RECEIVER_H
#define __RECEIVER_H

#include <hardtapi.h>
#include "config.h"
#include "input.h"

class BoomaReceiver {

    private:

        void CreateReceiver(ConfigOptions* opts, BoomaInput* input) {
            Create(opts, input->GetLastWriterConsumer());
        }

        int _sampleRate;

    protected:

        virtual HWriterConsumer<int16_t>* Create(ConfigOptions* opts, HWriterConsumer<int16_t>* previous) = 0;

        int GetSampleRate() {
            return _sampleRate;
        }

    public:

        BoomaReceiver(ConfigOptions* opts, BoomaInput* input):
            _sampleRate(opts->GetSampleRate()) {
            CreateReceiver(opts, input);
        };

        virtual ~BoomaReceiver() = default;

        virtual bool SetFrequency(long int frequency) = 0;
        virtual bool SetRfGain(int gain) = 0;

        virtual HWriterConsumer<int16_t>* GetLastWriterConsumer() = 0;
        virtual HWriterConsumer<int16_t>* GetSpectrumConsumer() = 0;
};

#endif
