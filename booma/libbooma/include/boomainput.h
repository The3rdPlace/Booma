#ifndef __INPUT_H
#define __INPUT_H

#include <hardtapi.h>
#include "configoptions.h"
#include "boomaexception.h"
#include "boomainputexception.h"
#include "booma.h"

class BoomaInput {

public:

    private:

        // Input and processor
        HReader<int16_t>* _inputReader;
        HStreamProcessor<int16_t>* _streamProcessor;
        HNetworkProcessor<int16_t>* _networkProcessor;

        // Decimation
        HProbe<int16_t>* _decimatorGainProbe;
        HGain<int16_t>* _decimatorGain;
        HProbe<int16_t>* _decimatorAgcProbe;
        HAgc<int16_t>* _decimatorAgc;
        HProbe<int16_t>* _ifMultiplierProbe;
        HIqMultiplier<int16_t>* _ifMultiplier;
        HProbe<int16_t>* _iqFirDecimatorProbe;
        HIqFirDecimator<int16_t>* _iqFirDecimator;
        HProbe<int16_t>* _iqDecimatorProbe;
        HIqDecimator<int16_t>* _iqDecimator;
        HProbe<int16_t>* _firDecimatorProbe;
        HFirDecimator<int16_t>* _firDecimator;
        HProbe<int16_t>* _decimatorProbe;
        HDecimator<int16_t>* _decimator;

        // Input filtering
        HIqFirFilter<int16_t>* _inputIqFirFilter;
        HFirFilter<int16_t>* _inputFirFilter;
        HProbe<int16_t>* _inputFirFilterProbe;

        // Dumping rf input
        HSplitter<int16_t>* _rfSplitter;
        HBreaker<int16_t>* _rfBreaker;
        HBufferedWriter<int16_t>* _rfBuffer;
        HWriter<int16_t>* _rfWriter;

        // Final consumer
        HWriterConsumer<int16_t>* _lastConsumer;

        int _virtualFrequency;
        int _hardwareFrequency;
        int _ifFrequency;

        HReader<int16_t>* SetInputReader(ConfigOptions* opts);
        bool SetReaderFrequencies(ConfigOptions *opts, int frequency);
        bool GetDecimationRate(int inputRate, int outputRate, int* first, int* second);
        HReader<int16_t>* SetDecimation(ConfigOptions* opts, HReader<int16_t>* reader);
        HWriterConsumer<int16_t>* SetInputFilter(ConfigOptions* options, HWriterConsumer<int16_t>* previous);
        HWriterConsumer<int16_t>* SetShift(ConfigOptions* options, HWriterConsumer<int16_t>* previous);
        HWriterConsumer<int16_t>* SetGain(ConfigOptions* options, HWriterConsumer<int16_t>* previous);

    public:

        class BoomaInputException : public BoomaException {

            public:

                BoomaInputException(std::string reason) : BoomaException(reason) {}
                std::string Type() { return "BoomaInputException"; }
        };

        BoomaInput(ConfigOptions* opts, bool* isTerminated);
        ~BoomaInput();

        HWriterConsumer<int16_t>* GetLastWriterConsumer() {
            return _lastConsumer;
        }

        void Run(int blocks = 0);

        void Halt();

        bool SetDumpRf(bool enabled);

        bool SetFrequency(ConfigOptions* opts, int frequency);

        int GetIfFrequency() {
            return _ifFrequency;
        }

        bool SetInputFilterWidth(ConfigOptions* opts, int width);
};

#endif
