#include <stdlib.h>
#include "config.h"
#include "receiver.h"

bool BoomaReceiver::SetFrequency(ConfigOptions* opts, long int frequency) {
    opts->SetFrequency(frequency);
    return SetFrequency(frequency);
}