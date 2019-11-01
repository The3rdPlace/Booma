#ifndef __RECEIVER_H
#define __RECEIVER_H

#include <hardtapi.h>

class BoomaReceiver {

    public:

        BoomaReceiver(ConfigOptions* opts, int sampleRate,  HWriterConsumer<int16_t>* previous, HWriter<int16_t>* next):
            _sampleRate(sampleRate) {};

        bool SetFrequency(ConfigOptions* opts, long int frequency);
        bool SetRfGain(ConfigOptions* opts, int gain);

    protected:

        int getSampleRate() {
            return _sampleRate;
        }

    private:

        int _sampleRate;

        virtual bool SetFrequency(long int frequency) = 0;
        virtual bool SetRfGain(int gain) = 0;
};

#endif
