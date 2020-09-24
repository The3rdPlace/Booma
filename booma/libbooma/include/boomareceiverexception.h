#ifndef __RECEIVEREXCEPTION_H
#define __RECEIVEREXCEPTION_H

#include <boomaexception.h>

class BoomaReceiverException : public BoomaException {

    public:

        BoomaReceiverException(std::string reason):
                BoomaException(reason) {}

        BoomaReceiverException(const char* reason):
            BoomaException(reason) {}

        std::string Type() {
            return "BoomaReceiverException";
        }
};

#endif
