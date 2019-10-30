#include "application.h"
#include "cwreceiver.h"

BoomaApplication::BoomaApplication(int argc, char** argv, bool verbose):
    _current(NULL)
{

    // Initialize the Hardt toolkit.
    // Set the last argument to 'true' to enable verbose output instead of logging to a local file
    HInit(std::string("Booma"), verbose);

    // Show library name and and Hardt version.
    HLog("booma: using Hardt %s", getversion().c_str());

    // Parse input arguments
    _opts = new ConfigOptions(argc, argv);

    // Set initial receiver
    SetReceiver();
}

bool BoomaApplication::SetReceiver() {

    // Todo: Stop existing receiver and close all readers/writers before creating new

    // Setup input
    if( !SetInput() ) {
        HError("Failed to configure input");
        exit(1);
    }

    // Setup output
    if( !SetOutput() ) {
        HError("Failed to configure output");
        exit(1);
    }

    if( !SetDumps() ) {
        HError("Failed to configure dumps");
        exit(1);
    }

    // Create receiver
    switch( _opts->GetReceiverModeType() ) {
        case CW:
            _receiver = new BoomaCwReceiver(_opts, _pcmSplitter->Consumer(), _audioSplitter);
            return true;
        default:
            std::cout << "Unknown receiver type defined" << std::endl;
            return false;
    }
}

bool BoomaApplication::SetInput() {

    // If we are a remote head, then initialize a network processor
    if( _opts->GetIsRemoteHead() ) {
        HLog("Initializing network processor with remote input at %s:%d", _opts->GetRemoteServer(), _opts->GetRemotePort());
        processor = new HNetworkProcessor<int16_t>(_opts->GetRemoteServer(), _opts->GetRemotePort(), BLOCKSIZE, &IsTerminated);
        return true;
    }

    // If we are a server for a remote head, then initialize the input and a network processor
    if( _opts->GetUseRemoteHead()) {
        HLog("Initializing network processor with local audio input device %d", _opts->GetInputAudioDevice());
        _inputReader = new HSoundcardReader<int16_t>(_opts->GetInputAudioDevice(), SAMPLERATE, 1, H_SAMPLE_FORMAT_INT_16, BLOCKSIZE);
        processor = new HNetworkProcessor<int16_t>(_opts->GetRemotePort(), _inputReader, BLOCKSIZE, &IsTerminated);
        return true;
    }

    // Otherwise configure a local input
    switch( _opts->GetInputSourceType() ) {
        case AUDIO_DEVICE:
            HLog("Initializing audio input device %d", _opts->GetInputAudioDevice());
            _inputReader = new HSoundcardReader<int16_t>(_opts->GetInputAudioDevice(), SAMPLERATE, 1, H_SAMPLE_FORMAT_INT_16, BLOCKSIZE);
            processor = new HStreamProcessor<int16_t>(_inputReader, BLOCKSIZE, &IsTerminated);
            return true;
        case SIGNAL_GENERATOR:
            HLog("Initializing signal generator at frequency %d", _opts->GetSignalGeneratorFrequency());
            _inputReader = new HSineGenerator<int16_t>(SAMPLERATE, _opts->GetSignalGeneratorFrequency(), 10);
            processor = new HStreamProcessor<int16_t>(_inputReader, BLOCKSIZE, &IsTerminated);
            return true;
        default:
            std::cout << "No input source type defined" << std::endl;
            return false;
    }
}

bool BoomaApplication::SetOutput() {

    // If we have a remote head, then do nothing
    if( _opts->GetUseRemoteHead() ) {
        HLog("Using remote head, no local output");
        return true;
    }

    // Otherwise initialize the audio output and the output gain control (volume)
    HLog("Initializing audio output device");
    if( _opts->GetOutputAudioDevice() == -1 ) {
        HLog("Writing output audio to /dev/null device");
        _audioWriter = NULL;
        _nullWriter = new HNullWriter<int16_t>();
        _outputWriter = new HGain<int16_t>(_nullWriter->Writer(), _opts->GetVolume(), BLOCKSIZE);
    }
    else
    {
        HLog("Initializing audio output device %d", _opts->GetOutputAudioDevice());
        _soundcardWriter = new HSoundcardWriter<int16_t>(_opts->GetOutputAudioDevice(), SAMPLERATE, 1, H_SAMPLE_FORMAT_INT_16, BLOCKSIZE);
        _nullWriter = NULL;
        _outputWriter = new HGain<int16_t>(_soundcardWriter, _opts->GetVolume(), BLOCKSIZE);
    }


    // Output configured
    return true;
}

bool BoomaApplication::SetDumps() {

    // If we have a remote head, then do nothing
    if( _opts->GetUseRemoteHead() ) {
    HLog("Using remote head, no local receiver chain");
        return true;
    }

    // Setup a splitter and a filewriter so that we can dump pcm data on request
    _pcmSplitter = new HSplitter<int16_t>(processor->Consumer());
    _pcmMute = new HMute<int16_t>(_pcmSplitter->Consumer(), !_opts->GetDumpPcm(), BLOCKSIZE);
    if( _opts->GetDumpFileFormat() == WAV ) {
        _pcmWriter = new HWavWriter<int16_t>("input.wav", H_SAMPLE_FORMAT_INT_16, 1, SAMPLERATE, _pcmMute->Consumer());
    } else {
        _pcmWriter = new HFileWriter<int16_t>("input.pcm", _pcmMute->Consumer());
    }

    // Setup a splitter and a filewriter so that we can dump audio data on request
    if( _opts->GetDumpFileFormat() == WAV ) {
        _audioWriter = new HWavWriter<int16_t>("output.wav", H_SAMPLE_FORMAT_INT_16, 1, SAMPLERATE);
    } else {
        _audioWriter = new HFileWriter<int16_t>("output.pcm");
    }
    _audioMute = new HMute<int16_t>(_audioWriter, !_opts->GetDumpAudio(), BLOCKSIZE);
    _audioSplitter = new HSplitter<int16_t>(_audioMute, _outputWriter);

    // Ready
    return true;
}

bool BoomaApplication::SetFrequency(long int frequency) {
    if( frequency >= SAMPLERATE / 2 || frequency <= 0 ) {
        return false;
    }
    return _receiver->SetFrequency(_opts, frequency);
}

long int BoomaApplication::GetFrequency() {
    return _opts->GetFrequency();
}

bool BoomaApplication::ChangeFrequency(int stepSize) {
    if( _opts->GetFrequency() + stepSize >= SAMPLERATE / 2 || _opts->GetFrequency() + stepSize <= 0 ) {
        return false;
    }
    return _receiver->SetFrequency(_opts, _opts->GetFrequency() + stepSize);
}

bool BoomaApplication::SetVolume(int volume) {
    if( volume >= SAMPLERATE / 2 || volume <= 0 ) {
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

bool BoomaApplication::ToggleDumpPcm() {
    _opts->SetDumpPcm(!_opts->GetDumpPcm());
    _pcmMute->SetMuted(!_opts->GetDumpPcm());
    return true;
}

bool BoomaApplication::ToggleDumpAudio() {
    _opts->SetDumpAudio(!_opts->GetDumpAudio());
    _audioMute->SetMuted(!_opts->GetDumpAudio());
    return true;
}

bool BoomaApplication::SetRfGain(int gain) {
    return _receiver->SetRfGain(_opts, gain);
}

bool BoomaApplication::ChangeRfGain(int stepSize) {
    if( _opts->GetRfGain() + stepSize > 100 || _opts->GetRfGain() + stepSize < 0 ) {
        return false;
    }
    return _receiver->SetRfGain(_opts, _opts->GetRfGain() + stepSize);
}

int BoomaApplication::GetRfGain() {
    return _opts->GetRfGain();
}
