#ifndef __AMRECEIVER_H
#define __AMRECEIVER_H

#include <hardtapi.h>

#include "booma.h"
#include "configoptions.h"
#include "boomareceiver.h"

class BoomaAmReceiver : public BoomaReceiver {

private:

    // Preprocessing
    HIqFirFilter<int16_t>* _inputFirFilter;

    // Receiver
    HIq2AbsConverter<int16_t>* _absConverter;
    HCollector<int16_t>* _collector;
    HBiQuadFilter<HLowpassBiQuad<int16_t>, int16_t>* _outputFilter;

    // Postprocessing
    // ...(empty)...

    bool IsDataTypeSupported(InputSourceDataType datatype) {
        switch( datatype ) {
            case InputSourceDataType::IQ_INPUT_SOURCE_DATA_TYPE: return true;
            case InputSourceDataType::I_INPUT_SOURCE_DATA_TYPE: return true;
            case InputSourceDataType::Q_INPUT_SOURCE_DATA_TYPE: return true;
            default: return false;
        }
    }

    HWriterConsumer<int16_t>* PreProcess(ConfigOptions* opts, HWriterConsumer<int16_t>* previous);
    HWriterConsumer<int16_t>* Receive(ConfigOptions* opts, HWriterConsumer<int16_t>* previous);
    HWriterConsumer<int16_t>* PostProcess(ConfigOptions* opts, HWriterConsumer<int16_t>* previous);

    bool SetInternalFrequency(ConfigOptions* opts, int frequency);

    void OptionChanged(ConfigOptions* opts, std::string name, int value) {}

    long GetDefaultFrequency(ConfigOptions* opts) {
        return (opts->GetOutputSampleRate() / 2) / 2;
    }

    bool IsFrequencySupported(ConfigOptions* opts, long frequency) {
        return true;
    }

public:

    BoomaAmReceiver(ConfigOptions* opts, int initialFrequency);
    ~BoomaAmReceiver();

    std::string GetName() {
        return "AM";
    }

    std::string GetOptionInfoString() { return ""; }
};

#endif
