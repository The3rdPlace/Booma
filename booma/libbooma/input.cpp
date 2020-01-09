#include "input.h"

BoomaInput::BoomaInput(ConfigOptions* opts, bool* isTerminated):
    _streamProcessor(NULL),
    _networkProcessor(NULL),
    _inputReader(NULL),
    _rfWriter(NULL),
    _rfSplitter(NULL),
    _rfMute(NULL) {

    // If we are a server for a remote head, then initialize the input and a network processor
    if( opts->GetUseRemoteHead()) {

        HLog("Creating input reader");
        if( !SetInputReader(opts) ) {
            throw new BoomaInputException("Unable to create input reader");
        }

        HLog("Initializing network processor with selected input device");
        _networkProcessor = new HNetworkProcessor<int16_t>(opts->GetRemotePort(), _inputReader, BLOCKSIZE, isTerminated);
        return;
    }

    // If we are a remote head, then initialize a network processor, otherwise configure a local input
    if( opts->GetIsRemoteHead() ) {
        HLog("Initializing network processor with remote input at %s:%d", opts->GetRemoteServer().c_str(), opts->GetRemotePort());
        _networkProcessor = new HNetworkProcessor<int16_t>(opts->GetRemoteServer().c_str(), opts->GetRemotePort(), BLOCKSIZE, isTerminated);
    }
    else {
        HLog("Creating input reader");
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
    _rfMute = new HMute<int16_t>(_rfSplitter->Consumer(), !opts->GetDumpRf(), BLOCKSIZE);
    std::string dumpfile = "INPUT_" + std::to_string(std::time(nullptr));
    if( opts->GetDumpFileFormat() == WAV ) {
        _rfWriter = new HWavWriter<int16_t>((dumpfile + ".wav").c_str(), H_SAMPLE_FORMAT_INT_16, 1, opts->GetSampleRate(), _rfMute->Consumer());
    } else {
        _rfWriter = new HFileWriter<int16_t>((dumpfile + ".pcm").c_str(), _rfMute->Consumer());
    }
}

BoomaInput::~BoomaInput() {

    if( _networkProcessor != NULL ) {
        delete _networkProcessor;
    }
    if( _streamProcessor != NULL ) {
        delete _streamProcessor;
    }

    delete _inputReader;
    delete _rfWriter;
    delete _rfSplitter;
    delete _rfMute;
}

bool BoomaInput::SetInputReader(ConfigOptions* opts) {

    // Select input reader
    switch( opts->GetInputSourceType() ) {
        case AUDIO_DEVICE:
            HLog("Initializing audio input device %d", opts->GetInputAudioDevice());
            _inputReader = new HSoundcardReader<int16_t>(opts->GetInputAudioDevice(), opts->GetSampleRate(), 1, H_SAMPLE_FORMAT_INT_16, BLOCKSIZE);
            break;
        case SIGNAL_GENERATOR:
            HLog("Initializing signal generator at frequency %d", opts->GetSignalGeneratorFrequency());
            // The generator amplitude is adjustes so that, with the scalefactor '10' for the signallevel meter,
            // and a default gain setting of '25' for the receivers RF gain, it should produce a S9 signal
            _inputReader = new HSineGenerator<int16_t>(opts->GetSampleRate(), opts->GetSignalGeneratorFrequency(), 200);
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
        default:
            throw new BoomaInputException("No input source specified");
    }
    return true;
}

bool BoomaInput::SetDumpRf(bool enabled) {
    _rfMute->SetMuted(enabled);
    return _rfMute->GetMuted();
}

void BoomaInput::Run(int blocks) {
    if( blocks > 0 ) {
        (_networkProcessor != NULL ? (HProcessor<int16_t>*) _networkProcessor : (HProcessor<int16_t>*) _streamProcessor)->Run(blocks);
    } else {
        (_networkProcessor != NULL ? (HProcessor<int16_t>*) _networkProcessor : (HProcessor<int16_t>*) _streamProcessor)->Run();
    }
}