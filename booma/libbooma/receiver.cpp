#include <stdlib.h>
#include "config.h"
#include "receiver.h"

bool BoomaReceiver::SetFrequency(ConfigOptions* opts, long int frequency) {
    opts->SetFrequency(frequency);
    return SetFrequency(frequency);
}

bool BoomaReceiver::SetRfGain(ConfigOptions* opts, int gain) {
    opts->SetRfGain(gain);
    return SetRfGain(gain);
}