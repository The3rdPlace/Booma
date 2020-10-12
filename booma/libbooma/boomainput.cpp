#include "boomainput.h"

BoomaInput::BoomaInput(ConfigOptions* opts, bool* isTerminated):
    _streamProcessor(nullptr),
    _networkProcessor(nullptr),
    _inputReader(nullptr),
    _rfWriter(nullptr),
    _rfSplitter(nullptr),
    _rfBreaker(nullptr),
    _rfBuffer(nullptr),
    _rfGain(nullptr),
    _rfGainProbe(nullptr),
    _ifMultiplier(nullptr),
    _ifMultiplierProbe(nullptr),
    _preGain(1) {

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
        _networkProcessor = new HNetworkProcessor<int16_t>(opts->GetRemoteDataPort(), opts->GetRemoteCommandPort(), _inputReader, BLOCKSIZE, isTerminated);
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
        _streamProcessor = new HStreamProcessor<int16_t>(_inputReader, BLOCKSIZE, isTerminated);
    }

    // Setup a splitter to split off rf dump and spectrum calculation
    HLog("Setting up input RF splitter");
    _rfSplitter = new HSplitter<int16_t>((_networkProcessor != NULL ? (HProcessor<int16_t>*) _networkProcessor : (HProcessor<int16_t>*) _streamProcessor)->Consumer());

    // Add a filewriter so that we can dump pcm data on request
    HLog("Adding RF output filewriter");
    _rfBreaker = new HBreaker<int16_t>(_rfSplitter->Consumer(), !opts->GetDumpRf(), BLOCKSIZE);
    _rfBuffer = new HBufferedWriter<int16_t>(_rfBreaker->Consumer(), BLOCKSIZE, opts->GetReservedBuffers(), opts->GetEnableBuffers());
    std::string dumpfile = "INPUT_" + std::to_string(std::time(nullptr));
    if( opts->GetDumpRfFileFormat() == WAV ) {
        _rfWriter = new HWavWriter<int16_t>((dumpfile + ".wav").c_str(), H_SAMPLE_FORMAT_INT_16, 1, opts->GetInputSampleRate(), _rfBuffer->Consumer());
    } else {
        _rfWriter = new HFileWriter<int16_t>((dumpfile + ".pcm").c_str(), _rfBuffer->Consumer());
    }

    // Add probe on input since it may be interesting what the raw input looks like
    _passthroughProbe = new HProbe<int16_t>("input_00_input", opts->GetEnableProbes());
    _passthrough = new HPassThrough<int16_t>(_rfSplitter->Consumer(), BLOCKSIZE, _passthroughProbe);

    // Add RF gain
    HLog("Setting up RF gain");
    _rfGainProbe = new HProbe<int16_t>("input_01_rf_gain", opts->GetEnableProbes());
    _rfGain = new HGain<int16_t>(_passthrough->Consumer(), opts->GetRfGain() * _preGain, BLOCKSIZE, _rfGainProbe);

    // If we use an RTL-SDR (or other downconverting devices) we may running with an offset from the requested
    // tuned frequency to avoid LO leaks
    if( opts->GetInputSourceType() == InputSourceType::RTLSDR && (opts->GetRtlsdrOffset() != 0 || opts->GetRtlsdrCorrection() != 0) ) {

        // IQ data is captured with the device center frequency set at a (configurable) distance from the actual
        // physical frequency that we want to capture. This avoids the LO injections that can be found many places
        // in the spectrum - a small prize for having such a powerfull sdr at this low pricepoint.!
        HLog("Setting up IF multiplier for RTL-SDR device");
        _ifMultiplierProbe = new HProbe<int16_t>("input_02_if_multiplier", opts->GetEnableProbes());
        _ifMultiplier = new HIqMultiplier<int16_t>(_rfGain->Consumer(), opts->GetInputSampleRate(), 0 - opts->GetRtlsdrOffset() - opts->GetRtlsdrCorrection() * 100, 10, BLOCKSIZE, _ifMultiplierProbe);
    }
}

BoomaInput::~BoomaInput() {

    SAFE_DELETE(_networkProcessor);
    SAFE_DELETE(_streamProcessor);

    SAFE_DELETE(_inputReader);
    SAFE_DELETE(_rfWriter);
    SAFE_DELETE(_rfSplitter);
    SAFE_DELETE(_rfBreaker);
    SAFE_DELETE(_rfBuffer);

    SAFE_DELETE(_passthrough);
    SAFE_DELETE(_rfGain);
    SAFE_DELETE(_ifMultiplier);

    SAFE_DELETE(_passthroughProbe);
    SAFE_DELETE(_rfGainProbe);
    SAFE_DELETE(_ifMultiplierProbe);
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
            // The generator amplitude is adjustes so that, with the scalefactor '10' for the signallevel meter,
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
                    _preGain = 1;
                    break;
                case InputSourceDataType::I_INPUT_SOURCE_DATA_TYPE:
                    HLog("Initializing RTL-2832 device %d for I(nphase) data", opts->GetInputDevice());
                    _inputReader = new HRtl2832Reader<int16_t>(opts->GetInputDevice(), opts->GetInputSampleRate(), HRtl2832::MODE::I_SAMPLES, 0, _hardwareFrequency, BLOCKSIZE, 0, opts->GetRtlsdrCorrection());
                    _preGain = 1;
                    break;
                case InputSourceDataType::Q_INPUT_SOURCE_DATA_TYPE:
                    HLog("Initializing RTL-2832 device %d for Q(uadrature) data", opts->GetInputDevice());
                    HLog("Offset %d Correction %d hardware %d", opts->GetRtlsdrOffset(), opts->GetRtlsdrCorrection(), _hardwareFrequency);
                    _inputReader = new HRtl2832Reader<int16_t>(opts->GetInputDevice(), opts->GetInputSampleRate(), HRtl2832::MODE::Q_SAMPLES, 0, _hardwareFrequency, BLOCKSIZE, 0, opts->GetRtlsdrCorrection());
                    _preGain = 1;
                    break;
                case InputSourceDataType::REAL_INPUT_SOURCE_DATA_TYPE:
                    HLog("Initializing RTL-2832 device %d for REAL data (positive part of IQ spectrum)", opts->GetInputDevice());
                    _inputReader = new HRtl2832Reader<int16_t>(opts->GetInputDevice(), opts->GetInputSampleRate(), HRtl2832::MODE::REAL_SAMPLES, 0, _hardwareFrequency, BLOCKSIZE, 0, opts->GetRtlsdrCorrection());
                    _preGain = 1 ;
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
    return true;
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

bool BoomaInput::SetReaderFrequencies(ConfigOptions *opts, int frequency) {
    _virtualFrequency = frequency;
    HLog("Input virtual frequency = %d", _virtualFrequency);

    switch( opts->GetInputSourceType() ) {
        case RTLSDR:
            _hardwareFrequency = frequency - opts->GetRtlsdrOffset();
            _ifFrequency = 0;
            break;
        default:
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
    switch( opts->GetInputSourceType() ) {
        case RTLSDR:
            HLog("Setting RTL-SDR center frequency = %d", _hardwareFrequency);
            return ((HRtl2832Reader<int16_t>*) _inputReader)->SetCenterFrequency(_hardwareFrequency);
        default:
            return true;
    }
}

int BoomaInput::SetRfGain(int gain) {

    if( gain > 10 || gain < 1 ) {
        return _rfGain->GetGain();
    } else {
        _rfGain->SetGain(gain);
        return _rfGain->GetGain();
    }
}