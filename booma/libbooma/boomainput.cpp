#include "boomainput.h"

BoomaInput::BoomaInput(ConfigOptions* opts, bool* isTerminated):
        _inputReader(nullptr),
        _rfWriter(nullptr),
        _rfSplitter(nullptr),
        _rfBreaker(nullptr),
        _rfBuffer(nullptr),
        _rfAgc(nullptr),
        _rfAgcProbe(nullptr),
        _networkProcessor(nullptr),
        _streamProcessor(nullptr),
        _decimatorGain(nullptr),
        _decimatorAgc(nullptr),
        _ifMultiplierProbe(nullptr),
        _iqFirDecimatorProbe(nullptr),
        _iqDecimatorProbe(nullptr),
        _firDecimatorProbe(nullptr),
        _decimatorProbe(nullptr),
        _decimatorGainProbe(nullptr),
        _decimatorAgcProbe(nullptr),
        _ifMultiplier(nullptr),
        _iqFirDecimator(nullptr),
        _iqDecimator(nullptr),
        _firDecimator(nullptr),
        _decimator(nullptr),
        _inputIqFirFilter(nullptr),
        _inputFirFilter(nullptr),
        _inputFirFilterProbe(nullptr) {

    // Set default frequencies
    HLog("Calculating initial internal frequencies");
    if( !SetReaderFrequencies(opts, opts->GetFrequency()) )
    {
        throw new BoomaInputException("Unable to configure initial frequencies");
    }

    // If we are a server for a remote head, then initialize the input and a network processor
    if( opts->GetUseRemoteHead()) {

        HLog("Creating input reader for remote head");
        HReader<int16_t>* reader = SetInputReader(opts);

        HLog("Setting decimation for high rate input");
        reader = SetDecimation(opts, reader);

        HLog("Initializing network processor with selected input device");
        _networkProcessor = new HNetworkProcessor<int16_t>(opts->GetRemoteDataPort(), opts->GetRemoteCommandPort(), reader, BLOCKSIZE, isTerminated);
        return;
    }

    // If we are a remote head, then initialize a network processor, otherwise configure a local input
    if( opts->GetIsRemoteHead() ) {
        HLog("Initializing network processor with remote input at %s:%d", opts->GetRemoteServer().c_str(), opts->GetRemoteDataPort());
        _networkProcessor = new HNetworkProcessor<int16_t>(opts->GetRemoteServer().c_str(), opts->GetRemoteDataPort(), opts->GetRemoteCommandPort(), BLOCKSIZE, isTerminated);
    }
    else {
        HLog("Creating input reader for local hardware device");
        HReader<int16_t>* reader = SetInputReader(opts);

        HLog("Setting decimation for high rate input");
        reader = SetDecimation(opts, reader);

        HLog("Initializing stream processor with selected input device");
        _streamProcessor = new HStreamProcessor<int16_t>(reader, BLOCKSIZE, isTerminated);
    }

    // Setup a splitter to split off rf dump and spectrum calculation
    HLog("Setting up input RF splitter");
    _rfSplitter = new HSplitter<int16_t>((_networkProcessor != nullptr ? (HProcessor<int16_t>*) _networkProcessor : (HProcessor<int16_t>*) _streamProcessor)->Consumer());

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

    // Add inputfilter, gain and optional zero-shift
    HLog("Setting input filter, rf gain and optional zero shift");
    HWriterConsumer<int16_t>* shift = SetShift(opts, _rfSplitter->Consumer());
    HWriterConsumer<int16_t>* input = SetInputFilter(opts, shift);
    _lastConsumer = SetGain(opts, input);
}

BoomaInput::~BoomaInput() {

    SAFE_DELETE(_streamProcessor);
    SAFE_DELETE(_networkProcessor);

    SAFE_DELETE(_decimatorGain);
    SAFE_DELETE(_decimatorAgc);
    SAFE_DELETE(_ifMultiplierProbe);
    SAFE_DELETE(_iqFirDecimatorProbe);
    SAFE_DELETE(_iqDecimatorProbe);
    SAFE_DELETE(_firDecimatorProbe);
    SAFE_DELETE(_decimatorProbe);
    SAFE_DELETE(_ifMultiplier);
    SAFE_DELETE(_iqFirDecimator);
    SAFE_DELETE(_iqDecimator);
    SAFE_DELETE(_firDecimator);
    SAFE_DELETE(_decimator);
    SAFE_DELETE(_decimatorGainProbe);
    SAFE_DELETE(_decimatorAgcProbe);
    SAFE_DELETE(_inputIqFirFilter);
    SAFE_DELETE(_inputFirFilter);
    SAFE_DELETE(_inputFirFilterProbe);

    SAFE_DELETE(_inputReader);
    SAFE_DELETE(_rfWriter);
    SAFE_DELETE(_rfSplitter);
    SAFE_DELETE(_rfBreaker);
    SAFE_DELETE(_rfBuffer);

    SAFE_DELETE(_rfAgc);
    SAFE_DELETE(_rfAgcProbe);
}

HReader<int16_t>* BoomaInput::SetInputReader(ConfigOptions* opts) {

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
                    HLog("Initializing RTL-2832 device %d for IQ data with offset %d and correction %d", opts->GetInputDevice(), opts->GetRtlsdrOffset(), opts->GetRtlsdrCorrection());
                    _inputReader = new HRtl2832Reader<int16_t>(opts->GetInputDevice(), opts->GetInputSampleRate(), HRtl2832::MODE::IQ_SAMPLES, 0, _hardwareFrequency, BLOCKSIZE, 0, opts->GetRtlsdrCorrection());
                    break;
                case InputSourceDataType::I_INPUT_SOURCE_DATA_TYPE:
                    HLog("Initializing RTL-2832 device %d for I(nphase) data with offset %d and correction %d", opts->GetInputDevice(), opts->GetRtlsdrOffset(), opts->GetRtlsdrCorrection());
                    _inputReader = new HRtl2832Reader<int16_t>(opts->GetInputDevice(), opts->GetInputSampleRate(), HRtl2832::MODE::I_SAMPLES, 0, _hardwareFrequency, BLOCKSIZE, 0, opts->GetRtlsdrCorrection());
                    break;
                case InputSourceDataType::Q_INPUT_SOURCE_DATA_TYPE:
                    HLog("Initializing RTL-2832 device %d for Q(uadrature) data with offset %d and correction %d", opts->GetInputDevice(), opts->GetRtlsdrOffset(), opts->GetRtlsdrCorrection());
                    _inputReader = new HRtl2832Reader<int16_t>(opts->GetInputDevice(), opts->GetInputSampleRate(), HRtl2832::MODE::Q_SAMPLES, 0, _hardwareFrequency, BLOCKSIZE, 0, opts->GetRtlsdrCorrection());
                    break;
                case InputSourceDataType::REAL_INPUT_SOURCE_DATA_TYPE:
                    HLog("Initializing RTL-2832 device %d for REAL data (positive part of IQ spectrum) with offset %d and correction %d", opts->GetInputDevice(), opts->GetRtlsdrOffset(), opts->GetRtlsdrCorrection());
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

    return _inputReader;
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
        _hardwareFrequency = (frequency + opts->GetShift()) - opts->GetRtlsdrOffset() + opts->GetRtlsdrAdjust();
        _ifFrequency = 0;
    } else {
        _hardwareFrequency = frequency + opts->GetShift();
        _ifFrequency = frequency;
    }

    HLog("Input hardware frequency = %d", _hardwareFrequency);
    HLog("Input IF frequency = %d", _ifFrequency);
    return true;
}

bool BoomaInput::SetFrequency(ConfigOptions* opts, int frequency) {

    // Calculate new IF and hardware frequencies
    if( !SetReaderFrequencies(opts, frequency) ) {
        HError("Failed to set input reader frequency %d", frequency);
        return false;
    }

    // If we have a bandpass filter as inputfilter (REAL input), then move it
    if( _inputFirFilter != nullptr ) {
        _inputFirFilter->SetCoefficients(HBandpassKaiserBessel<int16_t>(_ifFrequency - (opts->GetInputFilterWidth() / 2), _ifFrequency + (opts->GetInputFilterWidth() / 2), opts->GetOutputSampleRate(), 51, 50).Calculate(),51);
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
    if( gain != 0 ) {
        float g = gain > 0 ? gain : ((float) 1 / ((float) gain * (float) -1));
        _rfAgc->SetGain(g);
        return _rfAgc->GetGain();
    } else {
        _rfAgc->SetEnabled(true);
        return 0;
    }
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

HReader<int16_t>* BoomaInput::SetDecimation(ConfigOptions* opts, HReader<int16_t>* previous) {

    // Decimation not enabled or required
    if (opts->GetInputSampleRate() == opts->GetOutputSampleRate()) {
        HLog("No decimation needed");
        return previous;
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

    // Decimators require a tiny bit of gain to overcome the loss in the FIR filters
    if( opts->GetDecimatorGain() > 0 ) {
        HLog("Using fixed gain=%d before decimator", opts->GetDecimatorGain());
        _decimatorGainProbe = new HProbe<int16_t>("input_01_decimator_gain", opts->GetEnableProbes());
        _decimatorGain = new HGain<int16_t>(previous, opts->GetDecimatorGain(), BLOCKSIZE, _decimatorGainProbe);
    } else {
        HLog("Using agc at average value=2000 before decimator");
        _decimatorAgcProbe = new HProbe<int16_t>("input_02_decimator_agc", opts->GetEnableProbes());
        _decimatorAgc = new HAgc<int16_t>(previous, opts->GetDecimatorAgcLevel(), 50, BLOCKSIZE, 6, true, _decimatorAgcProbe);
    }

    // Decimation for IQ signals
    if(opts->GetInputSourceDataType() == IQ_INPUT_SOURCE_DATA_TYPE || opts->GetInputSourceDataType() == I_INPUT_SOURCE_DATA_TYPE || opts->GetInputSourceDataType() == Q_INPUT_SOURCE_DATA_TYPE ) {

        // First decimation stage - a FIR decimator dropping the samplerate while filtering out-ouf-band frequencies
        HLog("Creating FIR decimator with factor %d = %d -> %d with FIR filter size %d", firstFactor, opts->GetInputSampleRate(), opts->GetInputSampleRate() / firstFactor, opts->GetFirFilterSize());
        _iqFirDecimatorProbe = new HProbe<int16_t>("input_03_iq_fir_decimator", opts->GetEnableProbes());
        _iqFirDecimator = new HIqFirDecimator<int16_t>(
            (_decimatorGain != nullptr ? _decimatorGain : _decimatorAgc)->Reader(),
            firstFactor,
            HLowpassKaiserBessel<int16_t>(opts->GetDecimatorCutoff(), opts->GetInputSampleRate(), opts->GetFirFilterSize(),120).Calculate(),
            opts->GetFirFilterSize(),
            BLOCKSIZE,
            true,
            _iqFirDecimatorProbe);

        // Second decimation stage, if needed - a regular decimator dropping the samplerate to the output samplerate
        if (secondFactor > 1) {
            HLog("Creating decimator with factor %d = %d -> %d", secondFactor, opts->GetInputSampleRate() / firstFactor, opts->GetOutputSampleRate());
            _iqDecimatorProbe = new HProbe<int16_t>("input_04_iq_decimator", opts->GetEnableProbes());
            _iqDecimator = new HIqDecimator<int16_t>(
                    _iqFirDecimator->Reader(),
                    secondFactor,
                    BLOCKSIZE,
                    true,
                    _iqDecimatorProbe);
            return _iqDecimator->Reader();
        } else {
            return _iqFirDecimator->Reader();
        }
    }

    // Decimation for REAL signals
    else if(opts->GetInputSourceDataType() == REAL_INPUT_SOURCE_DATA_TYPE ) {

        // First decimation stage - a FIR decimator dropping the samplerate while filtering out-ouf-band frequencies
        HLog("Creating FIR decimator with factor %d = %d -> %d and FIR filter size %d", firstFactor, opts->GetInputSampleRate(), opts->GetInputSampleRate() / firstFactor, opts->GetFirFilterSize());
        _firDecimatorProbe = new HProbe<int16_t>("input_05_fir_decimator", opts->GetEnableProbes());
        _firDecimator = new HFirDecimator<int16_t>(
                (_decimatorGain != nullptr ? _decimatorGain : _decimatorAgc)->Reader(),
                firstFactor,
                HLowpassKaiserBessel<int16_t>(opts->GetDecimatorCutoff(), opts->GetInputSampleRate(), opts->GetFirFilterSize(),96).Calculate(),
                opts->GetFirFilterSize(),
                BLOCKSIZE,
                _firDecimatorProbe);

        // Second decimation stage, if needed - a regular decimator dropping the samplerate to the output samplerate
        if (secondFactor > 1) {
            HLog("Creating decimator with factor %d = %d -> %d", secondFactor, opts->GetInputSampleRate() / firstFactor, opts->GetOutputSampleRate());
            _decimatorProbe = new HProbe<int16_t>("input_06_decimator", opts->GetEnableProbes());
            _decimator = new HDecimator<int16_t>(_firDecimator->Reader(), 3, BLOCKSIZE, _decimatorProbe);
            return _decimator->Reader();
        } else {
            return _firDecimator->Reader();
        }
    }

    // Input datatype which can not be decimated. Decimation should not have been requested!
    else {
        throw new BoomaInputException("Receiver base decimation can not be applied to the given input datatype");
    }
}

HWriterConsumer<int16_t>* BoomaInput::SetInputFilter(ConfigOptions* opts, HWriterConsumer<int16_t>* previous) {

    // If we use an RT_SDR the input has been decimated and needs further cleanup
    if( opts->GetOriginalInputSourceType() == RTLSDR ) {

        // Add extra filter the removes (mostly) anything outside the FIR cutoff frequency
        _inputFirFilterProbe = new HProbe<int16_t>("input_07_inputfirfilter", opts->GetEnableProbes());
        _inputIqFirFilter = new HIqFirFilter<int16_t>(previous,
            HLowpassKaiserBessel<int16_t>(opts->GetInputFilterWidth(), opts->GetOutputSampleRate(), 51, 50).Calculate(),
            51, BLOCKSIZE, _inputFirFilterProbe);
        return _inputIqFirFilter->Consumer();
    } else {

        // Add extra filter the removes (mostly) anything outside the current frequency passband frequency
        _inputFirFilterProbe = new HProbe<int16_t>("input_07_inputfirfilter", opts->GetEnableProbes());
        _inputFirFilter = new HFirFilter<int16_t>(previous,
            HBandpassKaiserBessel<int16_t>(_ifFrequency - (opts->GetInputFilterWidth() / 2), _ifFrequency + (opts->GetInputFilterWidth() / 2), opts->GetOutputSampleRate(), 51, 50).Calculate(),
            51, BLOCKSIZE, _inputFirFilterProbe);
        return _inputFirFilter->Consumer();
    }

    return previous;
}

bool BoomaInput::SetInputFilterWidth(ConfigOptions* opts, int width) {

    // Change input filter width for an IQ fir input filter
    if(_inputIqFirFilter != nullptr ) {
        HLog("Setting new input filter width %d", width);
        _inputIqFirFilter->SetCoefficients(HLowpassKaiserBessel<int16_t>(width, opts->GetOutputSampleRate(), 51, 50).Calculate(), 51);
        return true;
    }

    // If we have a bandpass filter as inputfilter (REAL input), then move it
    if( _inputFirFilter != nullptr ) {
        _inputFirFilter->SetCoefficients(HBandpassKaiserBessel<int16_t>(_ifFrequency - (opts->GetInputFilterWidth() / 2), _ifFrequency + (opts->GetInputFilterWidth() / 2), opts->GetOutputSampleRate(), 51, 50).Calculate(),51);
        return true;
    }

    return false;
}

HWriterConsumer<int16_t>* BoomaInput::SetGain(ConfigOptions* opts, HWriterConsumer<int16_t>* previous) {

    // Add RF gain
    HLog("Setting up RF gain");
    _rfAgcProbe = new HProbe<int16_t>("input_08_rf_agc", opts->GetEnableProbes());
    _rfAgc = new HAgc<int16_t>(previous, opts->GetRfAgcLevel(), 10, BLOCKSIZE, 6, false, _rfAgcProbe);
    if( opts->GetRfGain() != 0 ) {
        float g = opts->GetRfGain() > 0 ? opts->GetRfGain() : ((float) 1 / ((float) opts->GetRfGain() * (float) -1));
        _rfAgc->SetGain(g);
    }
    return _rfAgc->Consumer();
}

HWriterConsumer<int16_t>* BoomaInput::SetShift(ConfigOptions* opts, HWriterConsumer<int16_t>* previous) {

    // If we use an RTL-SDR (or other downconverting devices) we may running with an offset from the requested
    // tuned frequency to avoid LO leaks
    if( opts->GetOriginalInputSourceType() == RTLSDR && (opts->GetRtlsdrOffset() != 0 || opts->GetRtlsdrCorrection() != 0) ) {

        // IQ data is captured with the device center frequency set at a (configurable) distance from the actual
        // physical frequency that we want to capture. This avoids the LO injections that can be found many places
        // in the spectrum - a small prize for having such a powerfull sdr at this low pricepoint.!
        HLog("Setting up IF multiplier for RTL-SDR device (shift %d)", 0 - opts->GetRtlsdrOffset() - (opts->GetRtlsdrCorrection() * opts->GetRtlsdrCorrectionFactor()));
        _ifMultiplierProbe = new HProbe<int16_t>("input_09_if_multiplier", opts->GetEnableProbes());
        _ifMultiplier = new HIqMultiplier<int16_t>(previous, opts->GetOutputSampleRate(), 0 - opts->GetRtlsdrOffset() - opts->GetRtlsdrCorrection() * opts->GetRtlsdrCorrectionFactor(), 10, BLOCKSIZE, _ifMultiplierProbe);

        return _ifMultiplier->Consumer();
    }

    return previous;
}

