#ifndef __INPUTREADEREXCEPTION_H
#define __INPUTREADEREXCEPTION_H

#include <boomaexception.h>

class BoomaInputReaderException : public BoomaException {

    public:

        BoomaInputReaderException(std::string reason):
                BoomaException(reason) {}

        BoomaInputReaderException(const char* reason):
            BoomaException(reason) {}

        std::string Type() {
            return "BoomaInputReaderException";
        }
};

#endif
