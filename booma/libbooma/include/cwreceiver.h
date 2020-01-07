#ifndef __CWRECEIVER_H
#define __CWRECEIVER_H

#include <hardtapi.h>

#include "booma.h"
#include "config.h"
#include "receiver.h"
#include "input.h"

class BoomaCwReceiver : public BoomaReceiver {

    private:

        HWriterConsumer<int16_t>* Create(ConfigOptions* opts, HWriterConsumer<int16_t>* previous);

        bool _enableProbes;

        HProbe<int16_t>* _passthroughProbe;
        HProbe<int16_t>* _highpassProbe;
        HProbe<int16_t>* _gainProbe;
        HProbe<int16_t>* _humfilterProbe;
        HProbe<int16_t>* _preselectProbe;
        HProbe<int16_t>* _agcProbe;
        HProbe<int16_t>* _multiplierProbe;
        HProbe<int16_t>* _bandpassProbe;
        HProbe<int16_t>* _postSelectProbe;

        HPassThrough<int16_t>* _passthrough;
        HBiQuadFilter<HHighpassBiQuad<int16_t>, int16_t>* _highpass;
        HGain<int16_t>* _gain;
        HCombFilter<int16_t>* _humfilter;
        HSplitter<int16_t>* _spectrum;
        HBiQuadFilter<HBandpassBiQuad<int16_t>, int16_t>* _preselect;
        HAgc<int16_t>* _agc;
        HMultiplier<int16_t>* _multiplier;
        HCascadedBiQuadFilter<int16_t>* _bandpass;
        HCascadedBiQuadFilter<int16_t>* _postSelect;

        static float _bandpassCoeffs[];

    public:

        BoomaCwReceiver(ConfigOptions* opts, BoomaInput* input):
            BoomaReceiver(opts, input),
            _enableProbes(opts->GetEnableProbes()) {}

        ~BoomaCwReceiver();

        HWriterConsumer<int16_t>* GetLastWriterConsumer() {
            return _postSelect->Consumer();
        }

        HWriterConsumer<int16_t>* GetSpectrumConsumer() {
            return _spectrum->Consumer();
        }

        bool SetFrequency(long int frequency);
        bool SetRfGain(int gain);
};

#endif
