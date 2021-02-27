#ifndef __INPUTREADER_CPP
#define __INPUTREADER_CPP

#include "boomainputreader.h"

bool BoomaInputReader::GetDecimationRate(int inputRate, int outputRate, int* first, int* second) {

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

void BoomaInputReader::SetDecimation(ConfigOptions* opts) {

    // Decimation not enabled or required
    if (opts->GetInputSampleRate() == opts->GetOutputSampleRate()) {
        HLog("No decimation needed");
        return;
    }

    // Sanity check
    if (opts->GetInputSampleRate() % opts->GetOutputSampleRate() != 0) {
        throw new BoomaInputReaderException("no integer divisor exists to decimate the input samplerate to the output samplerate");
    }

    // Get decimation factors
    int firstFactor;
    int secondFactor;
    if (!GetDecimationRate(opts->GetInputSampleRate(), opts->GetOutputSampleRate(), &firstFactor, &secondFactor)) {
        HError("No possible decimation factors to go from %d to %d", opts->GetInputSampleRate(), opts->GetOutputSampleRate());
        throw new BoomaInputReaderException("No possible decimation factors to go from the input samplerate to the output samplerate");
    }

    if( _reader_uint8 != nullptr ) {
        Setup8bitChain(opts, firstFactor, secondFactor);
    } else {
        Setup16bitChain(opts, firstFactor, secondFactor);
    }
}

void BoomaInputReader::Setup8bitChain(ConfigOptions* opts, int firstFactor, int secondFactor) {

    // If we use an RTL-SDR (or other downconverting devices) we may running with an offset from the requested
    // tuned frequency to avoid LO leaks
    if( opts->GetOriginalInputSourceType() == RTLSDR && (opts->GetRtlsdrOffset() != 0 || opts->GetRtlsdrCorrection() != 0) ) {

        // IQ data is captured with the device center frequency set at a (configurable) distance from the actual
        // physical frequency that we want to capture. This avoids the LO injections that can be found many places
        // in the spectrum - a small prize for having such a powerfull sdr at this low pricepoint.!
        HLog("Setting up IF multiplier for RTL-SDR device");
        _ifMultiplierProbe_uint8 = new HProbe<uint8_t>("inputreader_01_if_multiplier", opts->GetEnableProbes());
        _ifMultiplier_uint8 = new HIqMultiplier<uint8_t>(_reader_uint8, opts->GetInputSampleRate(), 0 - opts->GetRtlsdrOffset() - opts->GetRtlsdrCorrection() * 100, 10, BLOCKSIZE, _ifMultiplierProbe_uint8);
    }


    // Decimation for IQ signals
    if(opts->GetInputSourceDataType() == IQ_INPUT_SOURCE_DATA_TYPE || opts->GetInputSourceDataType() == I_INPUT_SOURCE_DATA_TYPE || opts->GetInputSourceDataType() == Q_INPUT_SOURCE_DATA_TYPE ) {

        // First decimation stage - a FIR decimator dropping the samplerate while filtering out-ouf-band frequencies
        HLog("Creating FIR decimator with factor %d = %d -> %d", firstFactor, opts->GetInputSampleRate(), opts->GetInputSampleRate() / firstFactor);
        _iqFirDecimatorProbe_uint8 = new HProbe<uint8_t>("inputreader_02_iq_fir_decimator", opts->GetEnableProbes());
        _iqFirDecimator_uint8 = new HIqFirDecimator<uint8_t>(
            _ifMultiplier_uint8 != nullptr ? _ifMultiplier_uint8->Reader() : _reader_uint8,
             firstFactor,
             HLowpassKaiserBessel<int16_t>(_cutOff, opts->GetInputSampleRate(), 15,120).Calculate(),
             15, BLOCKSIZE,
             true,
             _iqFirDecimatorProbe_uint8);

        // Extra Fir filter to remove signals outside of the passband
        HLog("Creating lowpass FIR filter");
        _iqFirFilterProbe_uint8 = new HProbe<uint8_t>("receiver_03_iq_fir_filter", opts->GetEnableProbes());
        _iqFirFilter_uint8 = new HIqFirFilter<uint8_t>(
                _iqFirDecimator_uint8->Reader(),
                HLowpassKaiserBessel<int16_t>(_cutOff, opts->GetInputSampleRate() / firstFactor,25, 120).Calculate(),
                25,
                BLOCKSIZE,
                _iqFirFilterProbe_uint8);

        // Second decimation stage, if needed - a regular decimator dropping the samplerate to the output samplerate
        if (secondFactor > 1) {
            HLog("Creating decimator with factor %d = %d -> %d", secondFactor, opts->GetInputSampleRate() / firstFactor, opts->GetOutputSampleRate());
            _iqDecimatorProbe_uint8 = new HProbe<uint8_t>("receiver_04_iq_decimator", opts->GetEnableProbes());
            _iqDecimator_uint8 = new HIqDecimator<uint8_t>(
                    _iqFirFilter_uint8->Reader(),
                    secondFactor,
                    BLOCKSIZE,
                    true,
                    _iqDecimatorProbe_uint8);
        }
    }
/*
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
        throw new BoomaInputReaderException("Receiver base decimation can not be applied to the given input datatype");
    }
    */
}

void BoomaInputReader::Setup16bitChain(ConfigOptions* opts, int firstFactor, int secondFactor) {

    // No 16 bit input reader currently needs decimation (it may change
    // if we add other sdr readers for 16 bit dongles)
    throw new BoomaInputReaderException("16 bit decimation is not supported");
}

#endif