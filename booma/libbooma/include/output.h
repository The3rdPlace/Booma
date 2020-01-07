#ifndef __OUTPUT_H
#define __OUTPUT_H

#include <hardtapi.h>
#include "config.h"
#include "receiver.h"

class BoomaOutput {

    public:

        BoomaOutput(ConfigOptions* opts, BoomaReceiver* receiver);
};

#endif
