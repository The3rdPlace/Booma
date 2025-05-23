#ifndef __CWRECEIVER_H
#define __CWRECEIVER_H

#include <hardtapi.h>

#include "booma.h"
#include "configoptions.h"
#include "boomareceiver.h"
#include "boomainput.h"

class BoomaCwReceiver : public BoomaReceiver {

    private:

        // Preprocessing
        HHumFilter<int16_t>* _humfilter;
        HIq2IConverter<int16_t>* _iq2IConverter;
        HIqMultiplier<int16_t>* _iqMultiplier;
        HBiQuadFilter<HBandpassBiQuad<int16_t>, int16_t>* _preselect;
        HGain<int16_t>* _passbandGain;
        HMultiplier<int16_t>* _ifMixer;

        // Receiver
        HCascadedBiQuadFilter<int16_t>* _ifFilter;
        HMultiplier<int16_t>* _beatToneMixer;
        HCascadedBiQuadFilter<int16_t>* _postSelect;

        // Postprocessing
        // ...(empty)...

        static float _bandpassCoeffs[6][20];
        static int _bandpassWidths[];
        static float _cwCoeffs[];

        bool IsDataTypeSupported(InputSourceDataType datatype) {
            switch( datatype ) {
                case InputSourceDataType::REAL_INPUT_SOURCE_DATA_TYPE: return true;
                case InputSourceDataType::IQ_INPUT_SOURCE_DATA_TYPE: return true;
                case InputSourceDataType::I_INPUT_SOURCE_DATA_TYPE: return true;
                case InputSourceDataType::Q_INPUT_SOURCE_DATA_TYPE: return true;
                default: return false;
            }
        }

        HWriterConsumer<int16_t>* PreProcess(ConfigOptions* opts, HWriterConsumer<int16_t>* previous);
        HWriterConsumer<int16_t>* Receive(ConfigOptions* opts, HWriterConsumer<int16_t>* previous);
        HWriterConsumer<int16_t>* PostProcess(ConfigOptions* opts, HWriterConsumer<int16_t>* previous);

        void OptionChanged(ConfigOptions* opts, std::string name, int value);

        bool SetInternalFrequency(ConfigOptions* opts, int frequency);
        bool SetRfGain(int gain);

        int GetIfOffset() {
            // Calculate mixer offsets due to if filter shifting
            return GetOption("Ifshift") * (_bandpassWidths[GetOption("Bandwidth")] / 4);
        }

        long GetDefaultFrequency(ConfigOptions* opts) {
            return (opts->GetOutputSampleRate() / 2) / 2;
        }

        bool IsFrequencySupported(ConfigOptions* opts, long frequency) {
            if( opts->GetInputSourceDataType() == REAL_INPUT_SOURCE_DATA_TYPE ) {
                // This receiver will not tune lower than 6KHz since we use 6KHz as
                // the internal IF in the heterodyne mixing stage.
                return frequency < opts->GetOutputSampleRate() / 2 && (frequency > 6000);
            } else {
                // With an rtlsdr source, the frequency can be almost anything
                return frequency >= 0;
            }
        }

    public:

        BoomaCwReceiver(ConfigOptions* opts, int initialFrequency);
        ~BoomaCwReceiver();

        std::string GetName() {
            return "CW";
        }

        std::string GetOptionInfoString();
};

#endif
