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

        HStreamProcessor<int16_t>* _streamProcessor;
        HNetworkProcessor<int16_t>* _networkProcessor;

        HReader<int16_t>* _inputReader;
        HWriter<int16_t>* _rfWriter;
        HSplitter<int16_t>* _rfSplitter;
        HBreaker<int16_t>* _rfBreaker;
        HBufferedWriter<int16_t>* _rfBuffer;
        HGain<int16_t>* _rfGain;
        HProbe<int16_t>* _rfGainProbe;

        // Decimation
        HProbe<int16_t>* _ifMultiplierProbe;
        HProbe<int16_t>* _iqFirDecimatorProbe;
        HProbe<int16_t>* _iqDecimatorProbe;
        HProbe<int16_t>* _firDecimatorProbe;
        HProbe<int16_t>* _decimatorProbe;

        HGain<int16_t>* _decimatorGain;
        HIqMultiplier<int16_t>* _ifMultiplier;
        HIqFirDecimator<int16_t>* _iqFirDecimator;
        HIqDecimator<int16_t>* _iqDecimator;
        HFirDecimator<int16_t>* _firDecimator;
        HDecimator<int16_t>* _decimator;

        HWriterConsumer<int16_t>* _lastConsumer;

        int _virtualFrequency;
        int _hardwareFrequency;
        int _ifFrequency;

        HReader<int16_t>* SetInputReader(ConfigOptions* opts);
        bool SetReaderFrequencies(ConfigOptions *opts, int frequency);
        bool GetDecimationRate(int inputRate, int outputRate, int* first, int* second);
        HReader<int16_t>* SetDecimation(ConfigOptions* opts, HReader<int16_t>* reader);
        HWriterConsumer<int16_t>* SetGainAndShift(ConfigOptions* options, HWriterConsumer<int16_t>* previous);

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

        int SetRfGain(int gain);

        int GetIfFrequency() {
            return _ifFrequency;
        }
};

#endif
