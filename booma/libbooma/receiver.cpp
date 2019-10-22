#include "receiver.h"

bool BoomaReceiver::SetFrequency(ConfigOptions* opts, long int frequency) {
    opts->setFrequency(frequency);
    return SetFrequency(frequency);
}