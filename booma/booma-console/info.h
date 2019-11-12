#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#include "booma.h"

#ifndef BOOMA_INFO_H
#define BOOMA_INFO_H

class Info {

    private:

        BoomaApplication* _app;
        void Spectrum(std::string name, int fSample, double* spectrum, int n, int frequencyMarker = 0);

    public:

        Info(BoomaApplication* app) {
            _app = app;
        }

        void GetInfo();
};

#endif
