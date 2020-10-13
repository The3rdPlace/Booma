#ifndef __RECEIVER_CPP
#define __RECEIVER_CPP

#include "boomareceiver.h"

HWriterConsumer<int16_t>* BoomaReceiver::Decimate(ConfigOptions* opts, HWriterConsumer<int16_t>* previous) {

    // Decimation not enabled or required
    if( !_decimate || opts->GetInputSampleRate() == opts->GetOutputSampleRate() ) {
        HLog("No decimation requested");
        return previous;
    }

    // Sanity check
    if( opts->GetInputSampleRate() % opts->GetOutputSampleRate() != 0 ) {
        throw new BoomaReceiverException("no integer divisor exists to decimate the input samplerate to the output samplerate");
    }

    // Get decimation factors
    int firstFactor;
    int secondFactor;
    if( !GetDecimationRate(opts->GetInputSampleRate(), opts->GetOutputSampleRate(), &firstFactor, &secondFactor) ) {
        HError("No possible decimation factors to go from %d to %d", opts->GetInputSampleRate(), opts->GetOutputSampleRate());
        throw new BoomaReceiverException("No possible decimation factors to go from the input samplerate to the output samplerate");
    }

    // Decimation for IQ signals
    if(opts->GetInputSourceDataType() == IQ_INPUT_SOURCE_DATA_TYPE || opts->GetInputSourceDataType() == I_INPUT_SOURCE_DATA_TYPE || opts->GetInputSourceDataType() == Q_INPUT_SOURCE_DATA_TYPE ) {

        // First decimation stage - a FIR decimator dropping the samplerate while filtering out-ouf-band frequencies
        HLog("Creating FIR decimator with factor %d = %d -> %d", firstFactor, opts->GetInputSampleRate(), opts->GetInputSampleRate() / firstFactor);
        _iqFirDecimatorProbe = new HProbe<int16_t>("receiver_01_iq_fir_decimator", opts->GetEnableProbes());
        _iqFirDecimator = new HIqFirDecimator<int16_t>(previous, firstFactor, HLowpassKaiserBessel<int16_t>(_cutOff, opts->GetInputSampleRate(), 15,120).Calculate(), 15, BLOCKSIZE, true, _iqFirDecimatorProbe);

        // Extra Fir filter to remove signals outside of the passband
        HLog("Creating lowpass FIR filter");
        _iqFirFilterProbe = new HProbe<int16_t>("receiver_02_iq_fir_filter", opts->GetEnableProbes());
        _iqFirFilter = new HIqFirFilter<int16_t>(_iqFirDecimator->Consumer(),HLowpassKaiserBessel<int16_t>(_cutOff, opts->GetInputSampleRate() / firstFactor,25, 120).Calculate(), 25, BLOCKSIZE, _iqFirFilterProbe);

        // Second decimation stage, if needed - a regular decimator dropping the samplerate to the output samplerate
        if (secondFactor > 1) {
            HLog("Creating decimator with factor %d = %d -> %d", secondFactor, opts->GetInputSampleRate() / firstFactor, opts->GetOutputSampleRate());
            _iqDecimatorProbe = new HProbe<int16_t>("receiver_03_iq_decimator", opts->GetEnableProbes());
            _iqDecimator = new HIqDecimator<int16_t>(_iqFirFilter->Consumer(), 3, BLOCKSIZE, true, _iqDecimatorProbe);
            return _iqDecimator->Consumer();
        } else {
            return _iqFirFilter->Consumer();
        }
    }

    // Decimation for REAL signals
    else if(opts->GetInputSourceDataType() == REAL_INPUT_SOURCE_DATA_TYPE ) {

        // First decimation stage - a FIR decimator dropping the samplerate while filtering out-ouf-band frequencies
        HLog("Creating FIR decimator with factor %d = %d -> %d", firstFactor, opts->GetInputSampleRate(), opts->GetInputSampleRate() / firstFactor);
        _firDecimatorProbe = new HProbe<int16_t>("receiver_01_fir_decimator", opts->GetEnableProbes());
        _firDecimator = new HFirDecimator<int16_t>(previous, firstFactor, HLowpassKaiserBessel<int16_t>(_cutOff, opts->GetInputSampleRate(), 15,96).Calculate(), 15, BLOCKSIZE, _firDecimatorProbe);

        // Extra Fir filter to remove signals outside of the 3Khz passband
        HLog("Creating lowpass FIR filter");
        _firFilterProbe = new HProbe<int16_t>("receiver_02_fir_filter", opts->GetEnableProbes());
        _firFilter = new HFirFilter<int16_t>(_firDecimator->Consumer(),HLowpassKaiserBessel<int16_t>(_cutOff, opts->GetInputSampleRate() / firstFactor,15, 96).Calculate(), 15, BLOCKSIZE, _firFilterProbe);

        // Second decimation stage, if needed - a regular decimator dropping the samplerate to the output samplerate
        if (secondFactor > 1) {
            HLog("Creating decimator with factor %d = %d -> %d", secondFactor, opts->GetInputSampleRate() / firstFactor, opts->GetOutputSampleRate());
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

bool BoomaReceiver::GetDecimationRate(int inputRate, int outputRate, int* first, int* second) {

    // Run through all possible factors for the current blocksize
    // The first decimator only accepts factors that divide cleanly
    // up into the BLOCKSIZE
    for( int i = BLOCKSIZE; i > 0; i-- ) {

        // Check if we can divide without remainer
        if( BLOCKSIZE % i == 0 ) {

            // Check if the factor also divides into a integer samplerate
            if( inputRate % i == 0 ) {

                // Did we hit the output samplerate already ?
                if( inputRate / i == outputRate ) {
                    *first = i;
                    *second = 1;
                    HLog("Setting first decimation factor to %d and second factor to %d", *first, *second);
                    return true;
                }

                // Only check if the intermediate rate is higher than the output rate
                int intermediate = inputRate / i;
                if( intermediate > outputRate ) {

                    // Run through all sensible factors for the second decimator.
                    // This decimator supports asymmetric decimation factors, so factors
                    // that do not divide cleanly up into BLOCKSIZE is also valid
                    for( int j = 1; j < BLOCKSIZE; j++ ) {
                        if( intermediate / j == outputRate ) {
                            *first = i;
                            *second = j;
                            HLog("Setting first decimation factor to %d and second factor to %d", *first, *second);
                            return true;
                        }
                    }
                }
            }
        }
    }
    return false;
}

#endif