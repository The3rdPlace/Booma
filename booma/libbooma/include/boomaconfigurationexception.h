#ifndef __CONFIGURATIONEXCEPTION_H
#define __CONFIGURATIONEXCEPTION_H

#include <boomaexception.h>

class BoomaConfigurationException : public BoomaException {

    public:

        BoomaConfigurationException(std::string reason):
                BoomaException(reason) {}

        BoomaConfigurationException(const char* reason):
            BoomaException(reason) {}

        std::string Type() {
            return "BoomaConfigurationException";
        }
};

#endif
