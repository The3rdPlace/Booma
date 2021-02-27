#include "boomainput.h"

BoomaInput::BoomaInput(ConfigOptions* opts, bool* isTerminated):
    _inputReader(nullptr),
    _rfWriter(nullptr),
    _rfSplitter(nullptr),
    _rfBreaker(nullptr),
    _rfBuffer(nullptr),
    _rfGain(nullptr),
    _rfGainProbe(nullptr),
    _networkProcessor(nullptr),
    _streamProcessor(nullptr),
    _ifMultiplierProbe(nullptr),
    _iqFirDecimatorProbe(nullptr),
    _iqFirFilterProbe(nullptr),
    _iqDecimatorProbe(nullptr),
    _firDecimatorProbe(nullptr),
    _firFilterProbe(nullptr),
    _decimatorProbe(nullptr),
    _ifMultiplier(nullptr),
    _iqFirDecimator(nullptr),
    _iqFirFilter(nullptr),
    _iqDecimator(nullptr),
    _firDecimator(nullptr),
    _firFilter(nullptr),
    _decimator(nullptr),
    _cutOff(3000) {

    // Set default frequencies
    HLog("Calculating initial internal frequencies");
    if( !SetReaderFrequencies(opts, opts->GetFrequency()) )
    {
        throw new BoomaInputException("Unable to configure initial frequencies");
    }

    // If we are a server for a remote head, then initialize the input and a network processor
    if( opts->GetUseRemoteHead()) {

        HLog("Creating input reader for remote head");
        if( !SetInputReader(opts) ) {
            throw new BoomaInputException("Unable to create input reader");
        }

        HLog("Initializing network processor with selected input device");
        _networkProcessor = new HNetworkProcessor<int16_t>(opts->GetRemoteDataPort(), opts->GetRemoteCommandPort(), GetInputReader(), BLOCKSIZE, isTerminated);
        return;
    }

    // If we are a remote head, then initialize a network processor, otherwise configure a local input
    if( opts->GetIsRemoteHead() ) {
        HLog("Initializing network processor with remote input at %s:%d", opts->GetRemoteServer().c_str(), opts->GetRemoteDataPort());
        _networkProcessor = new HNetworkProcessor<int16_t>(opts->GetRemoteServer().c_str(), opts->GetRemoteDataPort(), opts->GetRemoteCommandPort(), BLOCKSIZE, isTerminated);
    }
    else {
        HLog("Creating input reader for local hardware device");
        if( !SetInputReader(opts) ) {
            throw new BoomaInputException("Unable to create input reader");
        }

        HLog("Initializing stream processor with selected input device");
        _streamProcessor = new HStreamProcessor<int16_t>(GetInputReader(), BLOCKSIZE, isTerminated);
    }

    // Setup a splitter to split off rf dump and spectrum calculation
    HLog("Setting up input RF splitter");
    _rfSplitter = new HSplitter<int16_t>((_networkProcessor != NULL ? (HProcessor<int16_t>*) _networkProcessor : (HProcessor<int16_t>*) _streamProcessor)->Consumer());

    // Add a filewriter so that we can dump pcm data on request
    HLog("Adding RF output filewriter");
    _rfBreaker = new HBreaker<int16_t>(_rfSplitter->Consumer(), !opts->GetDumpRf(), BLOCKSIZE);
    _rfBuffer = new HBufferedWriter<int16_t>(_rfBreaker->Consumer(), BLOCKSIZE, opts->GetReservedBuffers(), opts->GetEnableBuffers());
    std::string dumpfile = "INPUT_" + (opts->GetDumpFileSuffix() == "" ? std::to_string(std::time(nullptr)) : opts->GetDumpFileSuffix());
    if( opts->GetDumpRfFileFormat() == WAV ) {
        _rfWriter = new HWavWriter<int16_t>((dumpfile + ".wav").c_str(), H_SAMPLE_FORMAT_INT_16, 1, opts->GetOutputSampleRate(), _rfBuffer->Consumer());
    } else {
        _rfWriter = new HFileWriter<int16_t>((dumpfile + ".pcm").c_str(), _rfBuffer->Consumer());
    }
}

BoomaInput::~BoomaInput() {

    SAFE_DELETE(_streamProcessor);
    SAFE_DELETE(_networkProcessor);

    SAFE_DELETE(_ifMultiplierProbe);
    SAFE_DELETE(_iqFirDecimatorProbe);
    SAFE_DELETE(_iqFirFilterProbe);
    SAFE_DELETE(_iqDecimatorProbe);
    SAFE_DELETE(_firDecimatorProbe);
    SAFE_DELETE(_firFilterProbe);
    SAFE_DELETE(_decimatorProbe);
    SAFE_DELETE(_ifMultiplier);
    SAFE_DELETE(_iqFirDecimator);
    SAFE_DELETE(_iqFirFilter);
    SAFE_DELETE(_iqDecimator);
    SAFE_DELETE(_firDecimator);
    SAFE_DELETE(_firFilter);
    SAFE_DELETE(_decimator);

    SAFE_DELETE(_inputReader);
    SAFE_DELETE(_rfWriter);
    SAFE_DELETE(_rfSplitter);
    SAFE_DELETE(_rfBreaker);
    SAFE_DELETE(_rfBuffer);

    SAFE_DELETE(_rfGain);
    SAFE_DELETE(_rfGainProbe);
}

bool BoomaInput::SetInputReader(ConfigOptions* opts) {

    // Select input reader
    switch( opts->GetInputSourceType() ) {
        case AUDIO_DEVICE:
            HLog("Initializing audio input device %d", opts->GetInputDevice());
            _inputReader = new HSoundcardReader<int16_t>(opts->GetInputDevice(), opts->GetInputSampleRate(), 1, H_SAMPLE_FORMAT_INT_16, BLOCKSIZE);
            break;
        case SIGNAL_GENERATOR:
            HLog("Initializing signal generator at frequency %d", opts->GetSignalGeneratorFrequency());
            // The generator amplitude is adjusted so that, with the scalefactor '10' for the signallevel meter,
            // and a default gain setting of '25' for the receivers RF gain, it should produce a S9 signal
            _inputReader = new HSineGenerator<int16_t>(opts->GetInputSampleRate(), opts->GetSignalGeneratorFrequency(), 200);
            break;
        case PCM_FILE:
            HLog("Initializing pcm file reader for input file %s", opts->GetPcmFile().c_str());
            _inputReader = new HFileReader<int16_t>(opts->GetPcmFile());
            break;
        case WAV_FILE:
            HLog("Initializing wav file reader for input file %s", opts->GetWavFile().c_str());
            _inputReader = new HWavReader<int16_t>(opts->GetWavFile().c_str());
            break;
        case SILENCE:
            HLog("Initializing nullreader");
            _inputReader = new HNullReader<int16_t>();
            break;
        case RTLSDR:
            switch(opts->GetInputSourceDataType()) {
                case InputSourceDataType::IQ_INPUT_SOURCE_DATA_TYPE:
                    HLog("Initializing RTL-2832 device %d for IQ data", opts->GetInputDevice());
                    _inputReader = new HRtl2832Reader<int16_t>(opts->GetInputDevice(), opts->GetInputSampleRate(), HRtl2832::MODE::IQ_SAMPLES, 0, _hardwareFrequency, BLOCKSIZE, 0, opts->GetRtlsdrCorrection());
                    break;
                case InputSourceDataType::I_INPUT_SOURCE_DATA_TYPE:
                    HLog("Initializing RTL-2832 device %d for I(nphase) data", opts->GetInputDevice());
                    _inputReader = new HRtl2832Reader<int16_t>(opts->GetInputDevice(), opts->GetInputSampleRate(), HRtl2832::MODE::I_SAMPLES, 0, _hardwareFrequency, BLOCKSIZE, 0, opts->GetRtlsdrCorrection());
                    break;
                case InputSourceDataType::Q_INPUT_SOURCE_DATA_TYPE:
                    HLog("Initializing RTL-2832 device %d for Q(uadrature) data", opts->GetInputDevice());
                    HLog("Offset %d Correction %d hardware %d", opts->GetRtlsdrOffset(), opts->GetRtlsdrCorrection(), _hardwareFrequency);
                    _inputReader = new HRtl2832Reader<int16_t>(opts->GetInputDevice(), opts->GetInputSampleRate(), HRtl2832::MODE::Q_SAMPLES, 0, _hardwareFrequency, BLOCKSIZE, 0, opts->GetRtlsdrCorrection());
                    break;
                case InputSourceDataType::REAL_INPUT_SOURCE_DATA_TYPE:
                    HLog("Initializing RTL-2832 device %d for REAL data (positive part of IQ spectrum)", opts->GetInputDevice());
                    _inputReader = new HRtl2832Reader<int16_t>(opts->GetInputDevice(), opts->GetInputSampleRate(), HRtl2832::MODE::REAL_SAMPLES, 0, _hardwareFrequency, BLOCKSIZE, 0, opts->GetRtlsdrCorrection());
                    break;
                default:
                    HError("Unknown input source datatype specified (%d)", opts->GetInputSourceDataType());
                    throw new BoomaInputException("Unknown input source datatype specified");
            }
            break;
        default:
            HError("Unknown input source type specified (%d)", opts->GetInputSourceType());
            throw new BoomaInputException("Unknown input source type specified");
    }

    // Setup decimation
    return SetDecimation(opts);
}

bool BoomaInput::SetDumpRf(bool enabled) {
    _rfBreaker->SetOff(!enabled);
    return !_rfBreaker->GetOff();
}

void BoomaInput::Run(int blocks) {
    if( blocks > 0 ) {
        (_networkProcessor != NULL ? (HProcessor<int16_t>*) _networkProcessor : (HProcessor<int16_t>*) _streamProcessor)->Run(blocks);
    } else {
        (_networkProcessor != NULL ? (HProcessor<int16_t>*) _networkProcessor : (HProcessor<int16_t>*) _streamProcessor)->Run();
    }
}

void BoomaInput::Halt() {
    (_networkProcessor != NULL ? (HProcessor<int16_t>*) _networkProcessor : (HProcessor<int16_t>*) _streamProcessor)->Halt();
}

bool BoomaInput::SetReaderFrequencies(ConfigOptions *opts, int frequency) {
    _virtualFrequency = frequency;
    HLog("Input virtual frequency = %d", _virtualFrequency);

    if( opts->GetOriginalInputSourceType() == RTLSDR ) {
        _hardwareFrequency = frequency - opts->GetRtlsdrOffset();
        _ifFrequency = 0;
    } else {
        _hardwareFrequency = frequency;
        _ifFrequency = frequency;
    }

    HLog("Input hardware frequency = %d", _hardwareFrequency);
    HLog("Input IF frequency = %d", _ifFrequency);
    return true;
}

bool BoomaInput::SetFrequency(ConfigOptions* opts, int frequency) {

    if( !SetReaderFrequencies(opts, frequency) ) {
        HError("Failed to set input reader frequency %d", frequency);
        return false;
    }

    // Device handling
    if( opts->GetInputSourceType() == RTLSDR || opts->GetOriginalInputSourceType() == RTLSDR ) {
        HLog("Setting RTL-SDR center frequency = %d", _hardwareFrequency);
        (_networkProcessor != NULL ? (HProcessor<int16_t>*) _networkProcessor : (HProcessor<int16_t>*) _streamProcessor)
                ->Command(H_COMMAND_CLASS::TUNER, H_COMMAND_OPCODE::SET_FREQUENCY, _hardwareFrequency);
        return true;
    } else {
        return true;
    }
}

int BoomaInput::SetRfGain(int gain) {
    _rfGain->SetGain(gain);
    return _rfGain->GetGain();
}

bool BoomaInput::GetDecimationRate(int inputRate, int outputRate, int* first, int* second) {

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

bool BoomaInput::SetDecimation(ConfigOptions* opts) {

    // Decimation not enabled or required
    if (opts->GetInputSampleRate() == opts->GetOutputSampleRate()) {
        HLog("No decimation needed");
        return true;
    }

    // Sanity check
    if (opts->GetInputSampleRate() % opts->GetOutputSampleRate() != 0) {
        throw new BoomaInputException("no integer divisor exists to decimate the input samplerate to the output samplerate");
    }

    // Get decimation factors
    int firstFactor;
    int secondFactor;
    if (!GetDecimationRate(opts->GetInputSampleRate(), opts->GetOutputSampleRate(), &firstFactor, &secondFactor)) {
        HError("No possible decimation factors to go from %d to %d", opts->GetInputSampleRate(), opts->GetOutputSampleRate());
        throw new BoomaInputException("No possible decimation factors to go from the input samplerate to the output samplerate");
    }

    // Add RF gain
    HLog("Setting up RF gain");
    _rfGainProbe = new HProbe<int16_t>("input_01_rf_gain", opts->GetEnableProbes());
    _rfGain = new HGain<int16_t>(_inputReader, opts->GetRfGain(), BLOCKSIZE, _rfGainProbe);

    // If we use an RTL-SDR (or other downconverting devices) we may running with an offset from the requested
    // tuned frequency to avoid LO leaks
    if( opts->GetOriginalInputSourceType() == RTLSDR && (opts->GetRtlsdrOffset() != 0 || opts->GetRtlsdrCorrection() != 0) ) {

        // IQ data is captured with the device center frequency set at a (configurable) distance from the actual
        // physical frequency that we want to capture. This avoids the LO injections that can be found many places
        // in the spectrum - a small prize for having such a powerfull sdr at this low pricepoint.!
        HLog("Setting up IF multiplier for RTL-SDR device (shift %d)", 0 - opts->GetRtlsdrOffset() - opts->GetRtlsdrCorrection());
        _ifMultiplierProbe = new HProbe<int16_t>("input_02_if_multiplier", opts->GetEnableProbes());
        _ifMultiplier = new HIqMultiplier<int16_t>(_rfGain->Reader(), opts->GetInputSampleRate(), 0 - opts->GetRtlsdrOffset() - opts->GetRtlsdrCorrection() * 150, 10, BLOCKSIZE, _ifMultiplierProbe);
    }

    // Decimation for IQ signals
    if(opts->GetInputSourceDataType() == IQ_INPUT_SOURCE_DATA_TYPE || opts->GetInputSourceDataType() == I_INPUT_SOURCE_DATA_TYPE || opts->GetInputSourceDataType() == Q_INPUT_SOURCE_DATA_TYPE ) {

        // First decimation stage - a FIR decimator dropping the samplerate while filtering out-ouf-band frequencies
        HLog("Creating FIR decimator with factor %d = %d -> %d", firstFactor, opts->GetInputSampleRate(), opts->GetInputSampleRate() / firstFactor);
        _iqFirDecimatorProbe = new HProbe<int16_t>("input_03_iq_fir_decimator", opts->GetEnableProbes());
        _iqFirDecimator = new HIqFirDecimator<int16_t>(
            _ifMultiplier != nullptr ? _ifMultiplier->Reader() : _rfGain->Reader(),
            firstFactor,
            HLowpassKaiserBessel<int16_t>(_cutOff * 2, opts->GetInputSampleRate(), 15,120).Calculate(),
            15, BLOCKSIZE,
            true,
            _iqFirDecimatorProbe);

        // Extra Fir filter to remove signals outside of the passband
        HLog("Creating lowpass FIR filter");
        _iqFirFilterProbe = new HProbe<int16_t>("input_04_iq_fir_filter", opts->GetEnableProbes());
        _iqFirFilter = new HIqFirFilter<int16_t>(
            _iqFirDecimator->Reader(),
            HLowpassKaiserBessel<int16_t>(_cutOff, opts->GetInputSampleRate() / firstFactor,25, 120).Calculate(),
            25,
            BLOCKSIZE,
            _iqFirFilterProbe);

        // Second decimation stage, if needed - a regular decimator dropping the samplerate to the output samplerate
        if (secondFactor > 1) {
            HLog("Creating decimator with factor %d = %d -> %d", secondFactor, opts->GetInputSampleRate() / firstFactor, opts->GetOutputSampleRate());
            _iqDecimatorProbe = new HProbe<int16_t>("input_05_iq_decimator", opts->GetEnableProbes());
            _iqDecimator = new HIqDecimator<int16_t>(
                    _iqFirFilter->Reader(),
                    secondFactor,
                    BLOCKSIZE,
                    true,
                    _iqDecimatorProbe);
        }
    }

    // Decimation for REAL signals
    else if(opts->GetInputSourceDataType() == REAL_INPUT_SOURCE_DATA_TYPE ) {

        // First decimation stage - a FIR decimator dropping the samplerate while filtering out-ouf-band frequencies
        HLog("Creating FIR decimator with factor %d = %d -> %d", firstFactor, opts->GetInputSampleRate(), opts->GetInputSampleRate() / firstFactor);
        _firDecimatorProbe = new HProbe<int16_t>("input_06_fir_decimator", opts->GetEnableProbes());
        _firDecimator = new HFirDecimator<int16_t>(_ifMultiplier != nullptr ? _ifMultiplier->Reader() : _inputReader, firstFactor, HLowpassKaiserBessel<int16_t>(_cutOff, opts->GetInputSampleRate(), 15,96).Calculate(), 15, BLOCKSIZE, _firDecimatorProbe);

        // Extra Fir filter to remove signals outside of the 3Khz passband
        HLog("Creating lowpass FIR filter");
        _firFilterProbe = new HProbe<int16_t>("input_07_fir_filter", opts->GetEnableProbes());
        _firFilter = new HFirFilter<int16_t>(_firDecimator->Consumer(),HLowpassKaiserBessel<int16_t>(_cutOff, opts->GetInputSampleRate() / firstFactor,15, 96).Calculate(), 15, BLOCKSIZE, _firFilterProbe);

        // Second decimation stage, if needed - a regular decimator dropping the samplerate to the output samplerate
        if (secondFactor > 1) {
            HLog("Creating decimator with factor %d = %d -> %d", secondFactor, opts->GetInputSampleRate() / firstFactor, opts->GetOutputSampleRate());
            _decimatorProbe = new HProbe<int16_t>("input_07_decimator", opts->GetEnableProbes());
            _decimator = new HDecimator<int16_t>(_firFilter->Consumer(), 3, BLOCKSIZE, _decimatorProbe);
        }
    }

    // Input datatype which can not be decimated. Decimation should not have been requested!
    else {
        throw new BoomaInputException("Receiver base decimation can not be applied to the given input datatype");
    }

    return true;
}
