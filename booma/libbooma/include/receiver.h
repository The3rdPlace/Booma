#ifndef __RECEIVER_H
#define __RECEIVER_H

#include <hardtapi.h>

class BoomaReceiver: public HWriterConsumer<int16_t> {

    private:

        int _sampleRate;

    protected:

        virtual HWriterConsumer<int16_t>* GetLastWriterConsumer() = 0;

        int GetSampleRate() {
            return _sampleRate;
        }

    public:

        BoomaReceiver(int sampleRate):
            _sampleRate(sampleRate) {};

        virtual ~BoomaReceiver() = default;

        virtual bool SetFrequency(long int frequency) = 0;
        virtual bool SetRfGain(int gain) = 0;

        void SetWriter (HWriter<int16_t> *writer) {
            GetLastWriterConsumer()->SetWriter(writer);
        }
};

#endif
