#ifndef __CWRECEIVER_H
#define __CWRECEIVER_H

#include <hardtapi.h>

#include "booma.h"
#include "config.h"
#include "receiver.h"

class BoomaCwReceiver : public BoomaReceiver {

    public:

        BoomaCwReceiver(int sampleRate, int frequency, int gain, HWriterConsumer<int16_t>* previous);

        bool SetFrequency(long int frequency);
        bool SetRfGain(int gain);

    private:

        HProbe<int16_t>* _gainProbe;
        HProbe<int16_t>* _preselectProbe;
        HProbe<int16_t>* _multiplierProbe;
        HProbe<int16_t>* _bandpassProbe;
        HProbe<int16_t>* _lowpassProbe;

        HHumFilter<int16_t>* _humFilter;

        HGain<int16_t>* _gain;
        HBiQuadFilter<HBandpassBiQuad<int16_t>, int16_t>* _preselect;
        HMultiplier<int16_t>* _multiplier;

        static float _bandpassCoeffs[];

        HCascadedBiQuadFilter<int16_t>* _bandpass;
        HBiQuadFilter<HLowpassBiQuad<int16_t>, int16_t>* _lowpass;

        HWriterConsumer<int16_t>* GetLastWriterConsumer() {
            return _lowpass->Consumer();
        }
};

#endif
