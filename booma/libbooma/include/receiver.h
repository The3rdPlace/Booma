#ifndef __RECEIVER_H
#define __RECEIVER_H

#include <hardtapi.h>

class BoomaReceiver {

    public:

        BoomaReceiver(int sampleRate,  HWriterConsumer<int16_t>* previous, HWriter<int16_t>* next):
            _sampleRate(sampleRate) {};

        virtual bool SetFrequency(long int frequency) = 0;
        virtual bool SetRfGain(int gain) = 0;

    protected:

        int GetSampleRate() {
            return _sampleRate;
        }

    private:

        int _sampleRate;
};

#endif
