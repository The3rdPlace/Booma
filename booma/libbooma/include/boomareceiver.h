#ifndef __RECEIVER_H
#define __RECEIVER_H

#include <hardtapi.h>
#include "configoptions.h"
#include "boomainput.h"
#include "boomadecoder.h"
#include "option.h"

#include "boomareceiverexception.h"

class BoomaReceiver {

    private:

        // Input gain/agc
        int _gainValue;
        HAgc<int16_t>* _rfAgc;
        HProbe<int16_t>* _rfAgcProbe;

        // Major receiver hooks
        HWriterConsumer<int16_t>* _preProcess;
        HWriterConsumer<int16_t>* _receive;
        HWriterConsumer<int16_t>* _postProcess;
        HSplitter<int16_t>* _decoder;

        std::vector<Option> _options;

        int _frequency;

        bool _hasBuilded;

        bool SetOption(ConfigOptions* opts, std::string name, int value);

    protected:

        virtual std::string GetName() = 0;

        virtual HWriterConsumer<int16_t>* PreProcess(ConfigOptions* opts, HWriterConsumer<int16_t>* previous) = 0;
        virtual HWriterConsumer<int16_t>* Receive(ConfigOptions* opts, HWriterConsumer<int16_t>* previous) = 0;
        virtual HWriterConsumer<int16_t>* PostProcess(ConfigOptions* opts, HWriterConsumer<int16_t>* previous) = 0;

        virtual bool IsDataTypeSupported(InputSourceDataType datatype) = 0;

        void RegisterOption(Option option) {
            HLog("Registering option %s", option.Name.c_str());
            _options.push_back(option);
        }

        virtual void OptionChanged(ConfigOptions* opts, std::string name, int value) = 0;

        virtual bool SetInternalFrequency(ConfigOptions* opts, int frequency) = 0;

        /**
         * Base class for all Booma receivers
         *
         * @param opts Receiver options
         * @param initialFrequency The initial frequency being received on
         */
        BoomaReceiver(ConfigOptions* opts, int initialFrequency):
            _hasBuilded(false),
            _frequency(initialFrequency),
            _rfAgc(nullptr),
            _rfAgcProbe(nullptr) {

            HLog("Creating BoomaReceiver with initial frequency %d", _frequency);
        }

        int GetRfAgcLevel(ConfigOptions* opts);

    public:

        virtual ~BoomaReceiver() {
            SAFE_DELETE(_rfAgc);
            SAFE_DELETE(_rfAgcProbe);
        }

        virtual int GetOutputFilterWidth() {
            return 4000;
        };

        std::vector<Option>* GetOptions() {
            return &_options;
        }

        int GetOption(std::string name);

        bool SetOption(ConfigOptions* opts, std::string name, std::string value);

        virtual std::string GetOptionInfoString() = 0;

        void Build(ConfigOptions* opts, BoomaInput* input, BoomaDecoder* decoder = NULL);

        bool SetFrequency(ConfigOptions* opts, int frequency) {
            _frequency = frequency;
            return SetInternalFrequency(opts, _frequency);
        }

        int GetFrequency() {
            return _frequency;
        }

        virtual long GetDefaultFrequency(ConfigOptions* opts) = 0;
        virtual bool IsFrequencySupported(ConfigOptions* opts, long frequency) = 0;

        HWriterConsumer<int16_t>* GetLastWriterConsumer() {
            return _decoder->Consumer();
        }

        int SetRfGain(int gain);

        int GetRfGain() {
            return _gainValue;
        }

        float GetRfAgcCurrentGain() {
            return _rfAgc->GetGain();
        }

        bool SetRfGainEnabled(bool enabled);
};

#endif
