#include "application.h"
#include "cwreceiver.h"

BoomaApplication::BoomaApplication(std::string appName, std::string appVersion, int argc, char** argv, bool verbose):
    _opts(NULL),
    _current(NULL),
    _inputReader(NULL),
    _outputWriter(NULL),
    _soundcardWriter(NULL),
    _nullWriter(NULL),
    _rfWriter(NULL),
    _audioWriter(NULL),
    _rfSplitter(NULL),
    _audioSplitter(NULL),
    _rfMute(NULL),
    _audioMute(NULL),
    _receiver(NULL),
    _isRunning(false)
{

    // Initialize the Hardt toolkit.
    // Set the last argument to 'true' to enable verbose output instead of logging to a local file
    HInit(std::string("Booma"), verbose);

    // Show library name and and Hardt version.
    HLog("booma: using Hardt %s", getversion().c_str());

    // Parse input arguments
    _opts = new ConfigOptions(appName, appVersion, argc, argv);

    // Set initial receiver
    ChangeReceiver();
}

BoomaApplication::~BoomaApplication() {
    if( _opts != NULL ) {
        delete _opts;
    }
}

bool BoomaApplication::ChangeReceiver(ReceiverModeType receiverModeType) {

    // Make sure that a running receiver has been shut down
    HLog("Shutting down a running receiver (should we have one)");
    Halt();

    // Register new receiver type
    HLog("Setting new receiver type");
    _opts->SetReceiverModeType(receiverModeType);

    // Configure new receiver
    HLog("Creating new receiver");
    if( !ChangeReceiver() ) {
        HError("Failed to create new receiver");
        return false;
    }
    return true;
}

bool BoomaApplication::ChangeReceiver() {

    // Reset all previous receiver components
    if( _inputReader != NULL ) {
        delete _inputReader;
        _inputReader = NULL;
    }
    if( _outputWriter  != NULL ) {
        delete _outputWriter;
        _outputWriter = NULL;
    }
    if( _soundcardWriter != NULL ) {
        delete _soundcardWriter;
        _soundcardWriter = NULL;
    }
    if( _nullWriter != NULL ) {
        delete _nullWriter;
        _nullWriter = NULL;
    }
    if( _rfWriter != NULL ) {
        delete _rfWriter;
        _rfWriter = NULL;
    }
    if( _audioWriter != NULL ) {
        delete _audioWriter;
        _audioWriter = NULL;
    }
    if( _rfSplitter != NULL ) {
        delete _rfSplitter;
        _rfSplitter = NULL;
    }
    if( _audioSplitter != NULL ) {
        delete _audioSplitter;
        _audioSplitter = NULL;
    }
    if( _rfMute != NULL ) {
        delete _rfMute;
        _rfMute = NULL;
    }
    if( _audioMute != NULL ) {
        delete _audioMute;
        _audioMute = NULL;
    }
    if( _receiver != NULL ) {
        delete _receiver;
        _receiver = NULL;
    }

    // Setup input
    if( !SetInput() ) {
        HError("Failed to configure input");
        exit(1);
    }

    // Create receiver
    if( !SetReceiver() ) {
        HError("Failed to configure receiver");
        exit(1);
    }

    // Setup output
    if( !SetOutput() ) {
        HError("Failed to configure output");
        exit(1);
    }

    // Complete input-receiver-output chain configured
    return true;
}

bool BoomaApplication::SetReceiver() {

    // If we have a remote head, then do nothing
    if( _opts->GetUseRemoteHead() ) {
        HLog("Using remote head, no local receiver");
        return true;
    }

    // Create receiver
    switch( _opts->GetReceiverModeType() ) {
        case CW:
            _receiver = new BoomaCwReceiver(_opts->GetSampleRate(), _opts->GetFrequency(), _opts->GetRfGain(), _rfSplitter->Consumer());
            return true;
        default:
            std::cout << "Unknown receiver type defined" << std::endl;
            return false;
    }
}

bool BoomaApplication::SetInput() {

    // If we are a server for a remote head, then initialize the input and a network processor
    if( _opts->GetUseRemoteHead()) {
        HLog("Initializing network processor with local audio input device %d", _opts->GetInputAudioDevice());
        _inputReader = new HSoundcardReader<int16_t>(_opts->GetInputAudioDevice(), _opts->GetSampleRate(), 1, H_SAMPLE_FORMAT_INT_16, BLOCKSIZE);
        processor = new HNetworkProcessor<int16_t>(_opts->GetRemotePort(), _inputReader, BLOCKSIZE, &IsTerminated);
        return true;
    }

    // If we are a remote head, then initialize a network processor, otherwise configure a local input
    if( _opts->GetIsRemoteHead() ) {
        HLog("Initializing network processor with remote input at %s:%d", _opts->GetRemoteServer().c_str(), _opts->GetRemotePort());
        processor = new HNetworkProcessor<int16_t>(_opts->GetRemoteServer().c_str(), _opts->GetRemotePort(), BLOCKSIZE, &IsTerminated);
    }
    else {
        switch( _opts->GetInputSourceType() ) {
            case AUDIO_DEVICE:
                HLog("Initializing audio input device %d", _opts->GetInputAudioDevice());
                _inputReader = new HSoundcardReader<int16_t>(_opts->GetInputAudioDevice(), _opts->GetSampleRate(), 1, H_SAMPLE_FORMAT_INT_16, BLOCKSIZE);
                break;
            case SIGNAL_GENERATOR:
                HLog("Initializing signal generator at frequency %d", _opts->GetSignalGeneratorFrequency());
                _inputReader = new HSineGenerator<int16_t>(_opts->GetSampleRate(), _opts->GetSignalGeneratorFrequency(), 10);
                break;
            case PCM_FILE:
                HLog("Initializing pcm file reader for input file %s", _opts->GetPcmFile().c_str());
                _inputReader = new HFileReader<int16_t>(_opts->GetPcmFile());
                break;
            default:
                std::cout << "No input source type defined" << std::endl;
                return false;
        }
        processor = new HStreamProcessor<int16_t>(_inputReader, BLOCKSIZE, &IsTerminated);
    }

    // Setup a splitter and a filewriter so that we can dump pcm data on request
    _rfSplitter = new HSplitter<int16_t>(processor->Consumer());
    _rfMute = new HMute<int16_t>(_rfSplitter->Consumer(), !_opts->GetDumpRf(), BLOCKSIZE);
    if( _opts->GetDumpFileFormat() == WAV ) {
        _rfWriter = new HWavWriter<int16_t>("input.wav", H_SAMPLE_FORMAT_INT_16, 1, _opts->GetSampleRate(), _rfMute->Consumer());
    } else {
        _rfWriter = new HFileWriter<int16_t>("input.pcm", _rfMute->Consumer());
    }

    // Read
    return true;
}

bool BoomaApplication::SetOutput() {

    // If we have a remote head, then do nothing
    if( _opts->GetUseRemoteHead() ) {
        HLog("Using remote head, no local output");
        return true;
    }

    // Setup a splitter and a filewriter so that we can dump audio data on request
    HLog("Setting up output audio splitter");
    _audioSplitter = new HSplitter<int16_t>(_receiver->Consumer());
    _audioMute = new HMute<int16_t>(_audioSplitter->Consumer(), !_opts->GetDumpAudio(), BLOCKSIZE);
    if( _opts->GetDumpFileFormat() == WAV ) {
        _audioWriter = new HWavWriter<int16_t>("audio.wav", H_SAMPLE_FORMAT_INT_16, 1, _opts->GetSampleRate(), _audioMute->Consumer());
    } else {
        _audioWriter = new HFileWriter<int16_t>("audio.pcm", _audioMute->Consumer());
    }

    // Initialize the audio output and the output gain control (volume)
    HLog("Initializing audio output");
    _outputWriter = new HGain<int16_t>(_audioSplitter->Consumer(), _opts->GetVolume(), BLOCKSIZE);
    if( _opts->GetOutputAudioDevice() == -1 ) {
        HLog("Writing output audio to /dev/null device");
        _soundcardWriter = NULL;
        _nullWriter = new HNullWriter<int16_t>(_outputWriter->Consumer());
    }
    else
    {
        HLog("Initializing audio output device %d", _opts->GetOutputAudioDevice());
        _soundcardWriter = new HSoundcardWriter<int16_t>(_opts->GetOutputAudioDevice(), _opts->GetSampleRate(), 1, H_SAMPLE_FORMAT_INT_16, BLOCKSIZE, _outputWriter->Consumer());
        _nullWriter = NULL;
    }

    // Output configured
    return true;
}

bool BoomaApplication::SetFrequency(long int frequency) {
    if( _receiver->SetFrequency(frequency) ) {
        _opts->SetFrequency(frequency);
        return true;
    }
    return false;
}

long int BoomaApplication::GetFrequency() {
    return _opts->GetFrequency();
}

bool BoomaApplication::ChangeFrequency(int stepSize) {
    if( _receiver->SetFrequency(_opts->GetFrequency() + stepSize) ) {
        _opts->SetFrequency(_opts->GetFrequency() + stepSize);
        return true;
    }
    return false;
}

bool BoomaApplication::SetVolume(int volume) {
    if( volume >= _opts->GetSampleRate() / 2 || volume <= 0 ) {
        return false;
    }
    _opts->SetVolume(volume);
    _outputWriter->SetGain(_opts->GetVolume());
    return true;
}

bool BoomaApplication::ChangeVolume(int stepSize) {
    if( _opts->GetVolume() + stepSize >= 1000 || _opts->GetVolume() + stepSize <= 0 ) {
        return false;
    }
    _opts->SetVolume(_opts->GetVolume() + stepSize);
    _outputWriter->SetGain(_opts->GetVolume());
    return true;
}

int BoomaApplication::GetVolume() {
    return _opts->GetVolume();
}

bool BoomaApplication::ToggleDumpRf() {
    _opts->SetDumpRf(!_opts->GetDumpRf());
    _rfMute->SetMuted(!_opts->GetDumpRf());
    return true;
}

bool BoomaApplication::GetDumpRf() {
    return !_rfMute->GetMuted();
}

bool BoomaApplication::ToggleDumpAudio() {
    _opts->SetDumpAudio(!_opts->GetDumpAudio());
    _audioMute->SetMuted(!_opts->GetDumpAudio());
    return true;
}

bool BoomaApplication::GetDumpAudio() {
    return !_audioMute->GetMuted();
}

bool BoomaApplication::SetRfGain(int gain) {
    if( _receiver->SetRfGain(gain) ) {
        _opts->SetRfGain(gain);
        return true;
    }
    return false;
}

bool BoomaApplication::ChangeRfGain(int stepSize) {
    if( _receiver->SetRfGain(_opts->GetRfGain() + stepSize) ) {
        _opts->SetRfGain(_opts->GetRfGain() + stepSize);
        return true;
    }
    return false;
}

int BoomaApplication::GetRfGain() {
    return _opts->GetRfGain();
}
