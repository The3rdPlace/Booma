#ifndef __AMRECEIVER_H
#define __AMRECEIVER_H

#include <hardtapi.h>

#include "booma.h"
#include "config.h"
#include "receiver.h"

class BoomaAmReceiver : public BoomaReceiver {

private:

    bool _enableProbes;

    HProbe<int16_t>* _passthroughProbe;
    HProbe<int16_t>* _highpassProbe;
    HProbe<int16_t>* _gainProbe;
    HProbe<int16_t>* _humfilterProbe;
    HProbe<int16_t>* _bandpassProbe;
    HProbe<int16_t>* _agcProbe;

    // Preprocessing
    HCombFilter<int16_t>* _humfilter;

    // Receiver
    HIq2AbsConverter<int16_t>* _conv;
    HCollector<int16_t>* _coll;
    HIqFirDecimator<int16_t>* _fdec;
    HIqDecimator<int16_t>* _dec;
    HPassThrough<int16_t>* _passthrough;
    HBiQuadFilter<HHighpassBiQuad<int16_t>, int16_t>* _highpass;
    HGain<int16_t>* _gain;
    HFirFilter<int16_t>* _bandpass;
    HAgc<int16_t>* _agc;

    bool IsDataTypeSupported(InputSourceDataType datatype) {
        switch( datatype ) {
            case InputSourceDataType::IQ: return true;
            default: return false;
        }
    }

    HWriterConsumer<int16_t>* PreProcess(ConfigOptions* opts, HWriterConsumer<int16_t>* previous);
    HWriterConsumer<int16_t>* Receive(ConfigOptions* opts, HWriterConsumer<int16_t>* previous);
    HWriterConsumer<int16_t>* PostProcess(ConfigOptions* opts, HWriterConsumer<int16_t>* previous);

    bool SetFrequency(int frequency);
    bool SetRfGain(int gain);

    void OptionChanged(std::string name, int value) {}

public:

    BoomaAmReceiver(ConfigOptions* opts, int initialFrequency, int initialRfGain);
    ~BoomaAmReceiver();

    std::string GetName() {
        return "AM";
    }

    std::string GetOptionInfoString() { return ""; }
};

#endif
