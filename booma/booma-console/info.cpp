#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <booma.h>

#include "info.h"

void Info::GetInfo(BoomaApplication app) {

    // Signallevel
    std::cout << "Average signallevel: S 0" << std::endl;
    std::cout << std::endl;
    std::cout << std::endl;

    // RF spectrum
    Spectrum("RF spectrum", 48000);
    std::cout << std::endl;

    // Audio spectrum
    Spectrum("Audio spectrum", 48000);
    std::cout << std::endl;

    // Metrics
    std::cout << "Incomming blocks: 0 = 0 bytes" << std::endl;
    std::cout << "Outgoing blocks: 0 = 0 bytes" << std::endl;
}

int Info::SignalLevel() {
    return 0;
}

void Info::Spectrum(std::string name, int fSample) {
    std::cout << ".[" << name << "]............................................." << std::endl;
    std::cout << ".                             *                       *   ." << std::endl;
    std::cout << ".                  *          *                       *   ." << std::endl;
    std::cout << ".         *        ***        *               **      *   ." << std::endl;
    std::cout << ".         *       *****      ***             ***      *   ." << std::endl;
    std::cout << "[0]....................[" << (fSample / 4) << "]..........................[" << (fSample / 2) << "]" << std::endl;
    std::cout << std::endl;
}