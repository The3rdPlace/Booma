#include "application.h"
#include "cwreceiver.h"

BoomaApplication::BoomaApplication(int argc, char** argv, bool verbose):
    current(NULL)
{

    // Initialize the Hardt toolkit.
    // Set the last argument to 'true' to enable verbose output instead of logging to a local file
    HInit(std::string("Booma"), verbose);

    // Show library name and and Hardt version.
    HLog("booma: using Hardt %s", getversion().c_str());

    // Parse input arguments
    _opts = new ConfigOptions(argc, argv);

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

    // Set initial receiver
    SetReceiver();
}

bool BoomaApplication::SetReceiver() {

    // Create receiver
    switch( _opts->GetReceiverModeType() ) {
        case CW:
            receiver = new BoomaCwReceiver(_opts, pcmSplitter->Consumer(), audioSplitter);
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
        inputReader = new HSoundcardReader<int16_t>(_opts->GetInputAudioDevice(), SAMPLERATE, 1, H_SAMPLE_FORMAT_INT_16, BLOCKSIZE);
        processor = new HNetworkProcessor<int16_t>(_opts->GetRemotePort(), inputReader, BLOCKSIZE, &IsTerminated);
        return true;
    }

    // Otherwise configure a local input
    switch( _opts->GetInputSourceType() ) {
        case AUDIO_DEVICE:
            HLog("Initializing audio input device %d", _opts->GetInputAudioDevice());
            inputReader = new HSoundcardReader<int16_t>(_opts->GetInputAudioDevice(), SAMPLERATE, 1, H_SAMPLE_FORMAT_INT_16, BLOCKSIZE);
            processor = new HStreamProcessor<int16_t>(inputReader, BLOCKSIZE, &IsTerminated);
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
    _soundcardWriter = new HSoundcardWriter<int16_t>(_opts->GetOutputAudioDevice(), SAMPLERATE, 1, H_SAMPLE_FORMAT_INT_16, BLOCKSIZE);
    outputWriter = new HGain<int16_t>(_soundcardWriter, _opts->GetVolume(), BLOCKSIZE);

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
    pcmSplitter = new HSplitter<int16_t>(processor->Consumer());
    pcmMute = new HMute<int16_t>(pcmSplitter->Consumer(), !_opts->GetDumpPcm(), BLOCKSIZE);
    if( _opts->GetDumpFileFormat() == WAV ) {
        pcmWriter = new HWavWriter<int16_t>("input.wav", H_SAMPLE_FORMAT_INT_16, 1, SAMPLERATE, pcmMute->Consumer());
    } else {
        pcmWriter = new HFileWriter<int16_t>("input.pcm", pcmMute->Consumer());
    }

    // Setup a splitter and a filewriter so that we can dump audio data on request
    if( _opts->GetDumpFileFormat() == WAV ) {
        audioWriter = new HWavWriter<int16_t>("output.wav", H_SAMPLE_FORMAT_INT_16, 1, SAMPLERATE);
    } else {
        audioWriter = new HFileWriter<int16_t>("output.pcm");
    }
    audioMute = new HMute<int16_t>(audioWriter, !_opts->GetDumpAudio(), BLOCKSIZE);
    audioSplitter = new HSplitter<int16_t>(audioMute, outputWriter);

    // Ready
    return true;
}

bool BoomaApplication::SetFrequency(long int frequency) {
    return receiver->SetFrequency(_opts, frequency);
}

bool BoomaApplication::ChangeFrequency(int stepSize) {
    if( _opts->GetFrequency() + stepSize >= SAMPLERATE / 2 || _opts->GetFrequency() + stepSize <= 0 ) {
        return false;
    }
    return receiver->SetFrequency(_opts, _opts->GetFrequency() + stepSize);
}

bool BoomaApplication::ChangeVolume(int stepSize) {
    if( _opts->GetVolume() + stepSize >= 500 || _opts->GetVolume() + stepSize <= 0 ) {
        return false;
    }
    _opts->SetVolume(_opts->GetVolume() + stepSize);
    outputWriter->SetGain(_opts->GetVolume());
    return true;
}
