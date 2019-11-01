#ifndef __CWRECEIVER_H
#define __CWRECEIVER_H

#include <hardtapi.h>

#include "booma.h"
#include "config.h"
#include "receiver.h"

class BoomaCwReceiver : public BoomaReceiver {

    public:

        BoomaCwReceiver(ConfigOptions* opts, HWriterConsumer<int16_t>* previous, HWriter<int16_t>* next);

        bool SetFrequency(long int frequency);
        bool SetRfGain(int gain);

    private:

        HHumFilter<int16_t>* _humFilter;

        HGain<int16_t>* _gain;
        HBiQuadFilter<HBandpassBiQuad<int16_t>, int16_t>* _preselect;
        HMultiplier<int16_t>* _multiplier;

        static float _bandpassCoeffs[];

        HCascadedBiQuadFilter<int16_t>* _bandpass;
        HBiQuadFilter<HLowpassBiQuad<int16_t>, int16_t>* _lowpass;
};

#endif
