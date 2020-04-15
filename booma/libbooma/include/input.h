#ifndef __INPUT_H
#define __INPUT_H

#include <hardtapi.h>
#include "config.h"
#include "boomaexception.h"
#include "booma.h"

class BoomaInput {

    private:

        HStreamProcessor<int16_t>* _streamProcessor;
        HNetworkProcessor<int16_t>* _networkProcessor;

        HReader<int16_t>* _inputReader;
        HWriter<int16_t>* _rfWriter;
        HSplitter<int16_t>* _rfSplitter;
        HMute<int16_t>* _rfMute;

        bool SetInputReader(ConfigOptions* opts);

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

        bool SetDumpRf(bool enabled);
};

#endif
