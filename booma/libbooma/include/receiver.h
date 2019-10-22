#ifndef __RECEIVER_H
#define __RECEIVER_H

#include <hardtapi.h>

class BoomaReceiver {

    public:

        BoomaReceiver(ConfigOptions* opts, HWriterConsumer<int16_t>* previous, HWriter<int16_t>* next) {};

        bool SetFrequency(ConfigOptions* opts, long int frequency);

    private:

        virtual bool SetFrequency(long int frequency) = 0;
};

#endif
