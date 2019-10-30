#include <stdlib.h>
#include "config.h"
#include "receiver.h"

bool BoomaReceiver::SetFrequency(ConfigOptions* opts, long int frequency) {
    if( frequency >= H_SAMPLE_RATE_48K / 2 || frequency <= 0 ) {
        return false;
    }
    opts->SetFrequency(frequency);
    return SetFrequency(frequency);
}

bool BoomaReceiver::SetRfGain(ConfigOptions* opts, int gain) {
    if( gain < 0 || gain > 200 ) {
        return false;
    }
    opts->SetRfGain(gain);
    return SetRfGain(gain);
}