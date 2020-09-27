#ifndef __RECEIVER_CPP
#define __RECEIVER_CPP

#include "boomareceiver.h"

HWriterConsumer<int16_t>* BoomaReceiver::Decimate(HWriterConsumer<int16_t>* previous) {

    if( !_decimate && _inputSamplerate > _outputSamplerate ) {
        HLog("No decimation requested");
        return previous;
    }

    // Sanity check
    if( _inputSamplerate % _outputSamplerate != 0 ) {
        throw new BoomaReceiverException("no integer divisor exists to decimate the input samplerate to the output samplerate");
    }

    // Todo: Calculate decimation-factors for all possible in-out samplerate combinations
    int firstFactor = 8;
    int secondFactor = 3;

    // First decimation stage - a FIR decimator dropping the samplerate while filtering out-ouf-band frequencies
    HLog("Creating FIR decimator with factor %d = %d -> %d", firstFactor, _inputSamplerate, _inputSamplerate / firstFactor);
    _firDecimator = new HIqFirDecimator<int16_t>(previous, firstFactor, HLowpassKaiserBessel<int16_t>( _cutOff, _inputSamplerate, 15, 96).Calculate(), 15, BLOCKSIZE);

    // Extra Fir filter to remove signals outside of the 3Khz passband
    HLog("Creating lowpass FIR filter");
    _firFilter = new HIqFirFilter<int16_t>(_firDecimator->Consumer(), HLowpassKaiserBessel<int16_t>(_cutOff, _inputSamplerate / firstFactor, 15,96).Calculate(), 15, BLOCKSIZE);

    // Second decimation stage, if needed - a regular decimator dropping the samplerate to the output samplerate
    if( secondFactor > 1 ) {
        HLog("Creating decimator with factor %d = %d -> %d", secondFactor, _inputSamplerate / firstFactor, _outputSamplerate);
        _decimator = new HIqDecimator<int16_t>(_firFilter->Consumer(), 3, BLOCKSIZE);
        return _decimator->Consumer();
    } else {
        return _firFilter->Consumer();
    }
}

#endif