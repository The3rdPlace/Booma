#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#ifndef BOOMA_INFO_H
#define BOOMA_INFO_H

class Info {

    private:

        int SignalLevel();
        void Spectrum(std::string name, int fSample);

    public:

        void GetInfo(BoomaApplication app);
};

#endif
