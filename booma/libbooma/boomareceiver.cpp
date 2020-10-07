#ifndef __RECEIVER_CPP
#define __RECEIVER_CPP

#include "boomareceiver.h"

HWriterConsumer<int16_t>* BoomaReceiver::Decimate(ConfigOptions* opts, HWriterConsumer<int16_t>* previous) {

    // Decimation not enabled or required
    if( !_decimate || _inputSamplerate == _outputSamplerate ) {
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

    // Decimation for IQ signals
    if( opts->GetInputSourceDataType() == IQ || opts->GetInputSourceDataType() == I || opts->GetInputSourceDataType() == Q ) {

        // First decimation stage - a FIR decimator dropping the samplerate while filtering out-ouf-band frequencies
        HLog("Creating FIR decimator with factor %d = %d -> %d", firstFactor, _inputSamplerate, _inputSamplerate / firstFactor);
        _iqFirDecimatorProbe = new HProbe<int16_t>("receiver_01_iq_fir_decimator", opts->GetEnableProbes());
        _iqFirDecimator = new HIqFirDecimator<int16_t>(previous, firstFactor, HLowpassKaiserBessel<int16_t>(_cutOff, _inputSamplerate, 51,50).Calculate(), 51, BLOCKSIZE, true, _iqFirDecimatorProbe);

        // Extra Fir filter to remove signals outside of the passband
        HLog("Creating lowpass FIR filter");
        _iqFirFilterProbe = new HProbe<int16_t>("receiver_02_iq_fir_filter", opts->GetEnableProbes());
        _iqFirFilter = new HIqFirFilter<int16_t>(_iqFirDecimator->Consumer(),HLowpassKaiserBessel<int16_t>(_cutOff, _inputSamplerate / firstFactor,51, 50).Calculate(), 51, BLOCKSIZE, _iqFirFilterProbe);

        // Second decimation stage, if needed - a regular decimator dropping the samplerate to the output samplerate
        if (secondFactor > 1) {
            HLog("Creating decimator with factor %d = %d -> %d", secondFactor, _inputSamplerate / firstFactor, _outputSamplerate);
            _iqDecimatorProbe = new HProbe<int16_t>("receiver_03_iq_decimator", opts->GetEnableProbes());
            _iqDecimator = new HIqDecimator<int16_t>(_iqFirFilter->Consumer(), 3, BLOCKSIZE, true, _iqDecimatorProbe);
            return _iqDecimator->Consumer();
        } else {
            return _iqFirFilter->Consumer();
        }
    }

    // Decimation for REAL signals
    else if( opts->GetInputSourceDataType() == REAL ) {

        // First decimation stage - a FIR decimator dropping the samplerate while filtering out-ouf-band frequencies
        HLog("Creating FIR decimator with factor %d = %d -> %d", firstFactor, _inputSamplerate, _inputSamplerate / firstFactor);
        _firDecimatorProbe = new HProbe<int16_t>("receiver_01_fir_decimator", opts->GetEnableProbes());
        _firDecimator = new HFirDecimator<int16_t>(previous, firstFactor, HLowpassKaiserBessel<int16_t>(_cutOff, _inputSamplerate, 15,96).Calculate(), 15, BLOCKSIZE, _firDecimatorProbe);

        // Extra Fir filter to remove signals outside of the 3Khz passband
        HLog("Creating lowpass FIR filter");
        _firFilterProbe = new HProbe<int16_t>("receiver_02_fir_filter", opts->GetEnableProbes());
        _firFilter = new HFirFilter<int16_t>(_firDecimator->Consumer(),HLowpassKaiserBessel<int16_t>(_cutOff, _inputSamplerate / firstFactor,15, 96).Calculate(), 15, BLOCKSIZE, _firFilterProbe);

        // Second decimation stage, if needed - a regular decimator dropping the samplerate to the output samplerate
        if (secondFactor > 1) {
            HLog("Creating decimator with factor %d = %d -> %d", secondFactor, _inputSamplerate / firstFactor, _outputSamplerate);
            _decimatorProbe = new HProbe<int16_t>("receiver_03_decimator", opts->GetEnableProbes());
            _decimator = new HDecimator<int16_t>(_firFilter->Consumer(), 3, BLOCKSIZE, _decimatorProbe);
            return _decimator->Consumer();
        } else {
            return _firFilter->Consumer();
        }

    }

    // Input datatype which can not be decimated. Decimation should not have been requested!
    else {
        throw new BoomaReceiverException("Receiver base decimation can not be applied to the given input datatype");
    }
}

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

#endif