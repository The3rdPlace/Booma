#ifndef __SSBRECEIVER_H
#define __SSBRECEIVER_H

#include <hardtapi.h>

#include "booma.h"
#include "configoptions.h"
#include "boomareceiver.h"

class BoomaSsbReceiver : public BoomaReceiver {

private:

    bool _enableProbes;

    HPassThrough<int16_t>* _pt;
    HProbe<int16_t>* _ptp;

    HProbe<int16_t>* _iqMultiplierProbe;
    HProbe<int16_t>* _iqFirFilterProbe;
    HProbe<int16_t>* _iq2IConverterProbe;
    HProbe<int16_t>* _translateByFourProbe;
    HProbe<int16_t>* _lowpassFilterProbe;

    // Preprocessing
    HIqMultiplier<int16_t>* _iqMultiplier;
    HIqFirFilter<int16_t>* _iqFirFilter;
    HIq2IConverter<int16_t>* _iq2IConverter;
    HTranslateByFour<int16_t>* _translateByFour;
    HBiQuadFilter<HLowpassBiQuad<int16_t>, int16_t>* _lowpassFilter;

    // Receiver
    // ...(empty)...

    // Postprocessing
    // ...(empty)...

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

    void OptionChanged(std::string name, int value) {}

public:

    BoomaSsbReceiver(ConfigOptions* opts, int initialFrequency);
    ~BoomaSsbReceiver();

    std::string GetName() {
        return "SSB";
    }

    std::string GetOptionInfoString() { return ""; }
};

#endif
