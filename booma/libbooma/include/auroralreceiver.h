#ifndef __AURORALRECEIVER_H
#define __AURORALRECEIVER_H

#include <hardtapi.h>

#include "booma.h"
#include "config.h"
#include "receiver.h"

class BoomaAuroralReceiver : public BoomaReceiver {

    private:

        bool _enableProbes;

        HProbe<int16_t>* _gainProbe;
        HProbe<int16_t>* _humfilterProbe;
        HProbe<int16_t>* _bandpassProbe;
        HProbe<int16_t>* _agcProbe;

        // Preprocessing
        HCombFilter<int16_t>* _humfilter;

        // Receiver
        HGain<int16_t>* _gain;
        HFirFilter<int16_t>* _bandpass;
        HAgc<int16_t>* _agc;

        bool IsDataTypeSupported(InputSourceDataType datatype) {
            switch( datatype ) {
                case InputSourceDataType::REAL: return true;
                default: return false;
            }
        }

        HWriterConsumer<int16_t>* PreProcess(ConfigOptions* opts, HWriterConsumer<int16_t>* previous);
        HWriterConsumer<int16_t>* Receive(ConfigOptions* opts, HWriterConsumer<int16_t>* previous);
        HWriterConsumer<int16_t>* PostProcess(ConfigOptions* opts, HWriterConsumer<int16_t>* previous);

        void OptionChanged(std::string name, int value) {}

        bool SetFrequency(int frequency);
        bool SetRfGain(int gain);

    public:

        BoomaAuroralReceiver(ConfigOptions* opts, int initialFrequency, int initialRfGain);
        ~BoomaAuroralReceiver();


        std::string GetName() {
            return "CW";
        }

        std::string GetOptionInfoString() { return ""; }

};

#endif
