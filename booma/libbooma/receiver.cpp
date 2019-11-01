#include <stdlib.h>
#include "config.h"
#include "receiver.h"

bool BoomaReceiver::SetFrequency(ConfigOptions* opts, long int frequency) {
    if( SetFrequency(frequency) ) {
        opts->SetFrequency(frequency);
        return true;
    }
    return false;
}

bool BoomaReceiver::SetRfGain(ConfigOptions* opts, int gain) {
    if( SetRfGain(gain) ) {
        opts->SetRfGain(gain);
        return true;
    }
    return false;
}