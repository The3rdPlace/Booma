#ifndef __AURORALRECEIVER_H
#define __AURORALRECEIVER_H

#include <hardtapi.h>

#include "booma.h"
#include "configoptions.h"
#include "boomareceiver.h"

class BoomaAuroralReceiver : public BoomaReceiver {

    private:

        bool _enableProbes;

        HProbe<int16_t>* _humfilterProbe;
        HProbe<int16_t>* _bandpassProbe;

        // Preprocessing
        HCombFilter<int16_t>* _humfilter;

        // Receiver
        HFirFilter<int16_t>* _bandpass;

        // Postprocessing
        // ...(empty)...

        bool IsDataTypeSupported(InputSourceDataType datatype) {
            switch( datatype ) {
                case InputSourceDataType::REAL_INPUT_SOURCE_DATA_TYPE: return true;
                default: return false;
            }
        }

        HWriterConsumer<int16_t>* PreProcess(ConfigOptions* opts, HWriterConsumer<int16_t>* previous);
        HWriterConsumer<int16_t>* Receive(ConfigOptions* opts, HWriterConsumer<int16_t>* previous);
        HWriterConsumer<int16_t>* PostProcess(ConfigOptions* opts, HWriterConsumer<int16_t>* previous);

        void OptionChanged(ConfigOptions* opts, std::string name, int value);

        bool SetInternalFrequency(ConfigOptions* opts, int frequency);
        bool SetRfGain(int gain);

        int GetOutputFilterWidth() {
            return 10000;
        }

    public:

        BoomaAuroralReceiver(ConfigOptions* opts, int initialFrequency);
        ~BoomaAuroralReceiver();


        std::string GetName() {
            return "CW";
        }

        std::string GetOptionInfoString() { return ""; }

};

#endif
