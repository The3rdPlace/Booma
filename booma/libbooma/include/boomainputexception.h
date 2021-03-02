#ifndef __INPUTEXCEPTION_H
#define __INPUTEXCEPTION_H

#include <boomaexception.h>

class BoomaInputException : public BoomaException {

    public:

        BoomaInputException(std::string reason):
                BoomaException(reason) {}

        BoomaInputException(const char* reason):
            BoomaException(reason) {}

        std::string Type() {
            return "BoomaInputException";
        }
};

#endif
