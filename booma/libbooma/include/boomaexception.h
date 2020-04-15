#ifndef __BOOMAEXCEPTION_H
#define __BOOMAEXCEPTION_H

#include <hardtapi.h>

class BoomaException : public std::exception {

    private:

        std::string _what;
        std::string _type;

    public:

        BoomaException(std::string reason):
            _what(reason) {
            HError(reason.c_str());
        }

        BoomaException(const char* reason):
            _what(reason) {
            HError(reason);
        }

        std::string What() {
            return _what;
        }

        virtual std::string Type() = 0;
};

#endif
