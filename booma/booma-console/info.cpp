#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <math.h>
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

    // Get latest averaged spectrum
    double spectrum[_app->GetRfFftSize()];
    int n = _app->GetRfSpectrum(spectrum);

    // Find maximum magnitude for 2 bins
    double max = 0;
    for( int i = 0; i < n; i++ ) {
        int magnitude = spectrum[i++];
        magnitude += spectrum[i];
        if( magnitude > max ) {
            max = magnitude;
        }
    }

    // Scaling
    double factor = 100000 / max;

    std::string level10;
    std::string level9;
    std::string level8;
    std::string level7;
    std::string level6;
    std::string level5;
    std::string level4;
    std::string level3;
    std::string level2;
    std::string level1;
    for( int i = 0; i < n; i++ )
    {
        int magnitude = spectrum[i++];
        magnitude += spectrum[i];
        magnitude *= factor;

        int level = (int) (magnitude) / 10000;

        level1 += level > 0 ? "*" : " ";
        level2 += level > 1 ? "*" : " ";
        level3 += level > 2 ? "*" : " ";
        level4 += level > 3 ? "*" : " ";
        level5 += level > 4 ? "*" : " ";
        level6 += level > 5 ? "*" : " ";
        level7 += level > 6 ? "*" : " ";
        level8 += level > 7 ? "*" : " ";
        level9 += level > 8 ? "*" : " ";
        level10 += level > 9 ? "*" : " ";
    }

    // Display spectrum
    std::string header = std::string("[" + name + "]");
    header.insert(header.length(), (n / 2) - header.length(), '.');
    std::cout << header << std::endl;
    std::cout << level10 << std::endl;
    std::cout << level9 << std::endl;
    std::cout << level8 << std::endl;
    std::cout << level7 << std::endl;
    std::cout << level6 << std::endl;
    std::cout << level5 << std::endl;
    std::cout << level4 << std::endl;
    std::cout << level3 << std::endl;
    std::cout << level2 << std::endl;
    std::cout << level1 << std::endl;
    std::string footer = "";
    footer.insert(0, (n / 2), '.');

    // Display current frequency (approximate)
    int fs = _app->GetSampleRate() / 2;
    int f = _app->GetFrequency();
    float q = (float) f / (float) fs;
    int pos = (n / 2) * q;
    footer[pos] = '>';
    footer[pos + 1] = '<';
    std::cout << footer << std::endl;
}
