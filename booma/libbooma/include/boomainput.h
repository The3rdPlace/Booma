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
        int _cutOff;
        HProbe<int16_t>* _ifMultiplierProbe;
        HProbe<int16_t>* _iqFirDecimatorProbe;
        HProbe<int16_t>* _iqFirFilterProbe;
        HProbe<int16_t>* _iqDecimatorProbe;
        HProbe<int16_t>* _firDecimatorProbe;
        HProbe<int16_t>* _firFilterProbe;
        HProbe<int16_t>* _decimatorProbe;

        HIqMultiplier<int16_t>* _ifMultiplier;
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
        bool SetDecimation(ConfigOptions* opts);

        HReader<int16_t>* GetInputReader() {
            if( _iqDecimator != nullptr ) {
                return _iqDecimator->Reader();
            } else if( _decimator != nullptr ) {
                return _decimator->Reader();
            } else {
                return _inputReader;
            }
        }

    public:

        class BoomaInputException : public BoomaException {

            public:

                BoomaInputException(std::string reason) : BoomaException(reason) {}
                std::string Type() { return "BoomaInputException"; }
        };

        BoomaInput(ConfigOptions* opts, bool* isTerminated);
        ~BoomaInput();

        HWriterConsumer<int16_t>* GetLastWriterConsumer() {
            return _rfSplitter->Consumer();
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
