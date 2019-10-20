#ifndef __CWRECEIVER_H
#define __CWRECEIVER_H

#include <hardtapi.h>

#include <booma.h>
#include "internals.h"
#include "receiver.h"

class BoomaCwReceiver : public BoomaReceiver {

    public:

        BoomaCwReceiver(ConfigOptions* configOptions, HWriterConsumer<int16_t>* previous, HWriter<int16_t>* next);

    private:

        HHumFilter<int16_t>* _humFilter;

        HGain<int16_t>* _gain;
        HBiQuadFilter<HHighpassBiQuad<int16_t>, int16_t>* _highpass;
        HMultiplier<int16_t>* _multiplier;

        static float _bandpassCoeffs[];

        HCascadedBiQuadFilter<int16_t>* _bandpass;
        HBiQuadFilter<HLowpassBiQuad<int16_t>, int16_t>* _lowpass;
        HGain<int16_t>* _volume;
        HFade<int16_t>* _fade;
};

#endif
