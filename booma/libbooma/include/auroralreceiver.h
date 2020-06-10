#ifndef __AURORALRECEIVER_H
#define __AURORALRECEIVER_H

#include <hardtapi.h>

#include "booma.h"
#include "config.h"
#include "receiver.h"
#include "input.h"

class BoomaAuroralReceiver : public BoomaReceiver {

    private:

        bool _enableProbes;

        HProbe<int16_t>* _passthroughProbe;
        HProbe<int16_t>* _highpassProbe;
        HProbe<int16_t>* _gainProbe;
        HProbe<int16_t>* _humfilterProbe;
        HProbe<int16_t>* _bandpassProbe;
        HProbe<int16_t>* _agcProbe;

        // Preprocessing
        HPassThrough<int16_t>* _passthrough;
        HBiQuadFilter<HHighpassBiQuad<int16_t>, int16_t>* _highpass;
        HGain<int16_t>* _gain;
        HCombFilter<int16_t>* _humfilter;

        // Receiver
        //HCascadedBiQuadFilter<int16_t>* _bandpass;
        HBiQuadFilter<HBandpassBiQuad<int16_t>, int16_t>* _bandpass;
        HAgc<int16_t>* _agc;

        static float _bandpassCoeffs[];

        HWriterConsumer<int16_t>* PreProcess(ConfigOptions* opts, HWriterConsumer<int16_t>* previous);
        HWriterConsumer<int16_t>* Receive(ConfigOptions* opts, HWriterConsumer<int16_t>* previous);
        HWriterConsumer<int16_t>* PostProcess(ConfigOptions* opts, HWriterConsumer<int16_t>* previous);

    public:

        BoomaAuroralReceiver(ConfigOptions* opts);
        ~BoomaAuroralReceiver();

        bool SetFrequency(long int frequency);
        bool SetRfGain(int gain);
};

#endif
