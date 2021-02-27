#ifndef __RECEIVER_CPP
#define __RECEIVER_CPP

#include "boomareceiver.h"

int BoomaReceiver::RfFftCallback(HFftResults* result, size_t length) {

    // Store the current spectrum in the output buffer
    memcpy((void*) _rfSpectrum, (void*) result->Spectrum, sizeof(double) * _rfFftSize / 2);
    return length;
}

int BoomaReceiver::AudioFftCallback(HFftResults* result, size_t length) {

    // Store the current spectrum in the output buffer
    memcpy((void*) _audioSpectrum, (void*) result->Spectrum, sizeof(double) * _audioFftSize / 2);
    return length;
}

int BoomaReceiver::GetRfSpectrum(double* spectrum) {
    memcpy((void*) spectrum, _rfSpectrum, sizeof(double) * _rfFftSize / 2);
    return _rfFftSize / 2;
}

int BoomaReceiver::GetRfFftSize() {
    return _rfFftSize / 2;
}

int BoomaReceiver::GetAudioFftSize() {
    return _audioFftSize / 2;
}

int BoomaReceiver::GetAudioSpectrum(double* spectrum) {
    memcpy((void*) spectrum, _audioSpectrum, sizeof(double) * _audioFftSize / 2);
    return _audioFftSize / 2;
}

bool BoomaReceiver::SetOption(ConfigOptions* opts, std::string name, int value){
    for( std::vector<Option>::iterator it = _options.begin(); it != _options.end(); it++ ) {
        if( (*it).Name == name ) {
            for( std::vector<OptionValue>::iterator valIt = (*it).Values.begin(); valIt != (*it).Values.end(); valIt++ ) {
                if( (*valIt).Value == value ) {

                    // Discard repeated setting of the same value
                    if( (*it).CurrentValue == value ) {
                        HLog("Value is the same as the current value, discarding option set");
                        return true;
                    }

                    // Set current value
                    (*it).CurrentValue = value;

                    // Update the stored copy of receiver options
                    std::map<std::string, std::string> optionsMap;
                    for( std::vector<Option>::iterator optIt = _options.begin(); optIt != _options.end(); optIt++ ) {
                        for( std::vector<OptionValue>::iterator optValIt = (*optIt).Values.begin(); optValIt != (*optIt).Values.end(); optValIt++ ) {
                            if( (*optValIt).Value == (*optIt).CurrentValue ) {
                                optionsMap[(*optIt).Name] = (*optValIt).Name;
                                continue;
                            }
                        }
                    }
                    opts->SetReceiverOptionsFor(GetName(), optionsMap);

                    // Report the change to the receiver implementation
                    if( _hasBuilded ) {
                        OptionChanged(opts, name, value);
                    }

                    // Receiver option set
                    return true;
                }
            }
            HError("Attempt to internal set receiver option '%s' to non-existing value %d", name.c_str(), value);
            return false;
        }
    }
    HError("Attempt to internal set non-existing receiver option '%s' to value '%d'", name.c_str(), value);
    return false;
};
#endif