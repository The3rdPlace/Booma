#ifndef __RECEIVER_H
#define __RECEIVER_H

#include "internals.h"
#include <hardtapi.h>

class BoomaReceiver {

    public:

        BoomaReceiver(ConfigOptions* configOptions, HWriterConsumer<int16_t>* previous, HWriter<int16_t>* next) {};
};

#endif
