#ifndef __INPUT_H
#define __INPUT_H

#include <hardtapi.h>
#include "configoptions.h"
#include "boomaexception.h"
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
        HIqTranslateByFour<int16_t>* _ifshift;

        int _virtualFrequency;
        int _hardwareFrequency;
        int _ifFrequency;

        int _preGain;

        bool SetInputReader(ConfigOptions* opts);
        bool SetReaderFrequencies(ConfigOptions *opts, int frequency);

    public:

        class BoomaInputException : public BoomaException {

            public:

                BoomaInputException(std::string reason) : BoomaException(reason) {}
                std::string Type() { return "BoomaInputException"; }
        };

        BoomaInput(ConfigOptions* opts, bool* isTerminated);
        ~BoomaInput();

        HWriterConsumer<int16_t>* GetLastWriterConsumer() {
            return _ifshift->Consumer();
        }

        void Run(int blocks = 0);

        bool SetDumpRf(bool enabled);

        bool SetFrequency(ConfigOptions* opts, int frequency);

        int SetRfGain(int gain);

        int GetIfFrequency() {
            return _ifFrequency;
        }
};

#endif
