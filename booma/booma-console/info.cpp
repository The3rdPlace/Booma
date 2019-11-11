#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <booma.h>

#include "info.h"
#include <hardtapi.h>

void Info::GetInfo() {

    // Signallevel
    std::cout << "Average signallevel: " << _app->GetSignalLevel() << " dB" << std::endl;
    std::cout << std::endl;
    std::cout << std::endl;

    // RF spectrum
    Spectrum("RF spectrum", 48000);
    std::cout << std::endl;

    // Audio spectrum
    //Spectrum("Audio spectrum", 48000);
    //std::cout << std::endl;
}

void Info::Spectrum(std::string name, int fSample) {

    double spectrum[_app->GetRfFftSize()];
    int n = _app->GetRfSpectrum(spectrum);

    double max = 0;
    int maxI = 0;
    for( int i = 0; i < n; i++ ) {
        if( spectrum[i] > max ) {
            maxI = i;
            max = spectrum[i];
        }
    }
    std::cout << max << " @ " << maxI << std::endl;

    

    /*std::cout << ".[" << name << "]............................................." << std::endl;
    std::cout << ".                             *                       *   ." << std::endl;
    std::cout << ".                  *          *                       *   ." << std::endl;
    std::cout << ".         *        ***        *               **      *   ." << std::endl;
    std::cout << ".         *       *****      ***             ***      *   ." << std::endl;
    std::cout << "[0]....................[" << (fSample / 4) << "]..........................[" << (fSample / 2) << "]" << std::endl;
    std::cout << std::endl;*/
}
