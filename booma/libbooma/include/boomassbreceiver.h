#ifndef __SSBRECEIVER_H
#define __SSBRECEIVER_H

#include <hardtapi.h>

#include "booma.h"
#include "configoptions.h"
#include "boomareceiver.h"

class BoomaSsbReceiver : public BoomaReceiver {

    private:

        bool _enableProbes;

        HProbe<int16_t>* _inputFirFilterProbe;
        HProbe<int16_t>* _iqMultiplierProbe;
        HProbe<int16_t>* _iqFirFilterProbe;
        HProbe<int16_t>* _basebandMultiplierProbe;
        HProbe<int16_t>* _lowpassFilterProbe;
        HProbe<int16_t>* _iqAdderProbe;

        // Preprocessing
        HIqFirFilter<int16_t>* _inputFirFilter;
        HIqMultiplier<int16_t>* _iqMultiplier;
        HIqFirFilter<int16_t>* _iqFirFilter;
        HIqMultiplier<int16_t>* _basebandMultiplier;
        HBiQuadFilter<HLowpassBiQuad<int16_t>, int16_t>* _lowpassFilter;
        HIqAddOrSubtractConverter<int16_t>* _iqAdder;
        HCollector<int16_t>* _collector;

        // Receiver
        // ...(empty)...

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

        void OptionChanged(ConfigOptions* opts, std::string name, int value);

        long GetDefaultFrequency(ConfigOptions* opts) {
            return (opts->GetOutputSampleRate() / 2) / 2;
        }

        bool IsFrequencySupported(ConfigOptions* opts, long frequency) {
            return frequency < (opts->GetOutputSampleRate() / 2);
        }

    public:

        BoomaSsbReceiver(ConfigOptions* opts, int initialFrequency);
        ~BoomaSsbReceiver();

        std::string GetName() {
            return "SSB";
        }

        std::string GetOptionInfoString();
};

#endif
