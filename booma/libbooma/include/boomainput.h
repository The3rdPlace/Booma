#ifndef __INPUT_H
#define __INPUT_H

#include <hardtapi.h>
#include "configoptions.h"
#include "boomaexception.h"
#include "boomainputexception.h"
#include "boomaprocessor.h"
#include "boomainputreader.h"
#include "booma.h"

class BoomaInput {

public:

    private:

        BoomaProcessor* _processor;

        BoomaInputReader* _inputReader;
        HWriter<int16_t>* _rfWriter;
        HSplitter<int16_t>* _rfSplitter;
        HBreaker<int16_t>* _rfBreaker;
        HBufferedWriter<int16_t>* _rfBuffer;
        HPassThrough<int16_t>* _passthrough;
        HGain<int16_t>* _rfGain;
        HIqMultiplier<int16_t>* _ifMultiplier;

        HProbe<int16_t>* _passthroughProbe;
        HProbe<int16_t>* _rfGainProbe;
        HProbe<int16_t>* _ifMultiplierProbe;
        HProbe<int16_t>* _iqFirDecimatorProbe;
        HProbe<int16_t>* _iqFirFilterProbe;
        HProbe<int16_t>* _iqDecimatorProbe;
        HProbe<int16_t>* _firDecimatorProbe;
        HProbe<int16_t>* _firFilterProbe;
        HProbe<int16_t>* _decimatorProbe;

        bool _decimate;
        int _cutOff;
        HIqFirDecimator<int16_t>* _iqFirDecimator;
        HIqFirFilter<int16_t>* _iqFirFilter;
        HIqDecimator<int16_t>* _iqDecimator;
        HFirDecimator<int16_t>* _firDecimator;
        HFirFilter<int16_t>* _firFilter;
        HDecimator<int16_t>* _decimator;

        int _virtualFrequency;
        int _hardwareFrequency;
        int _ifFrequency;

        bool SetInputReader(ConfigOptions* opts);
        bool SetReaderFrequencies(ConfigOptions *opts, int frequency);

        bool GetDecimationRate(int inputRate, int outputRate, int* first, int* second);
        HWriterConsumer<int16_t>* Decimate(ConfigOptions* opts, HWriterConsumer<int16_t>* previous);

    public:

        class BoomaInputException : public BoomaException {

            public:

                BoomaInputException(std::string reason) : BoomaException(reason) {}
                std::string Type() { return "BoomaInputException"; }
        };

        BoomaInput(ConfigOptions* opts, bool* isTerminated);
        ~BoomaInput();

        HWriterConsumer<int16_t>* GetLastWriterConsumer() {
            return _ifMultiplier != nullptr ? _ifMultiplier->Consumer() : _rfGain->Consumer();
        }

        void Run(int blocks = 0);

        void Halt();

        bool SetDumpRf(bool enabled);

        bool SetFrequency(ConfigOptions* opts, int frequency);

        int SetRfGain(int gain);

        int GetIfFrequency() {
            return _ifFrequency;
        }
};

#endif
