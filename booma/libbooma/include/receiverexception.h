#ifndef __RECEIVEREXCEPTION_H
#define __RECEIVEREXCEPTION_H

#include <boomaexception.h>

class ReceiverException : public BoomaException {

    public:

        ReceiverException(std::string reason):
                BoomaException(reason) {}

        ReceiverException(const char* reason):
            BoomaException(reason) {}

        std::string Type() {
            return "ReceiverException";
        }
};

#endif
