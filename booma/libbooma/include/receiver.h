#ifndef __RECEIVER_H
#define __RECEIVER_H

#include <hardtapi.h>
#include "config.h"

class BoomaReceiver {

    private:

        int _sampleRate;

    protected:

        int GetSampleRate() {
            return _sampleRate;
        }

    public:

        BoomaReceiver(int sampleRate):
            _sampleRate(sampleRate) {};

        virtual ~BoomaReceiver() = default;

        virtual bool SetFrequency(long int frequency) = 0;
        virtual bool SetRfGain(int gain) = 0;

        virtual HWriterConsumer<int16_t>* GetLastWriterConsumer() = 0;
        virtual HWriterConsumer<int16_t>* GetSpectrumConsumer() = 0;
};

#endif
