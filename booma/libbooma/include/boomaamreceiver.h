#ifndef __AMRECEIVER_H
#define __AMRECEIVER_H

#include <hardtapi.h>

#include "booma.h"
#include "configoptions.h"
#include "boomareceiver.h"

class BoomaAmReceiver : public BoomaReceiver {

private:

    bool _enableProbes;

    HProbe<int16_t>* _humfilterProbe;

    // Preprocessing
    HIq2AbsConverter<int16_t>* _conv;
    HCollector<int16_t>* _coll;
    HIqFirDecimator<int16_t>* _fdec;
    HIqDecimator<int16_t>* _dec;

    // Receiver
    // ...(empty)...

    // Postprocessing
    // ...(empty)...

    bool IsDataTypeSupported(InputSourceDataType datatype) {
        switch( datatype ) {
            case InputSourceDataType::IQ: return true;
            case InputSourceDataType::REAL: return true;
            default: return false;
        }
    }

    HWriterConsumer<int16_t>* PreProcess(ConfigOptions* opts, HWriterConsumer<int16_t>* previous);
    HWriterConsumer<int16_t>* Receive(ConfigOptions* opts, HWriterConsumer<int16_t>* previous);
    HWriterConsumer<int16_t>* PostProcess(ConfigOptions* opts, HWriterConsumer<int16_t>* previous);

    bool SetFrequency(int frequency);

    void OptionChanged(std::string name, int value) {}

public:

    BoomaAmReceiver(ConfigOptions* opts, int initialFrequency);
    ~BoomaAmReceiver();

    std::string GetName() {
        return "AM";
    }

    std::string GetOptionInfoString() { return ""; }
};

#endif
