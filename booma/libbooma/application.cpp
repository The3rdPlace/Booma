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
    switch( _opts->getReceiverModeType() ) {
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
    if( _opts->getIsRemoteHead() ) {
        HLog("Initializing network processor with remote input at %s:%d", _opts->getRemoteServer(), _opts->getRemotePort());
        processor = new HNetworkProcessor<int16_t>(_opts->getRemoteServer(), _opts->getRemotePort(), BLOCKSIZE, &IsTerminated);
        return true;
    }

    // If we are a server for a remote head, then initialize the input and a network processor
    if( _opts->getUseRemoteHead()) {
        HLog("Initializing network processor with local audio input device %d", _opts->getInputAudioDevice());
        inputReader = new HSoundcardReader<int16_t>(_opts->getInputAudioDevice(), SAMPLERATE, 1, H_SAMPLE_FORMAT_INT_16, BLOCKSIZE);
        processor = new HNetworkProcessor<int16_t>(_opts->getRemotePort(), inputReader, BLOCKSIZE, &IsTerminated);
        return true;
    }

    // Otherwise configure a local input
    switch( _opts->getInputSourceType() ) {
        case AUDIO_DEVICE:
            HLog("Initializing audio input device %d", _opts->getInputAudioDevice());
            inputReader = new HSoundcardReader<int16_t>(_opts->getInputAudioDevice(), SAMPLERATE, 1, H_SAMPLE_FORMAT_INT_16, BLOCKSIZE);
            processor = new HStreamProcessor<int16_t>(inputReader, BLOCKSIZE, &IsTerminated);
            return true;
        default:
            std::cout << "No input source type defined" << std::endl;
            return false;
    }
}

bool BoomaApplication::SetOutput() {

    // If we have a remote head, then do nothing
    if( _opts->getUseRemoteHead() ) {
        HLog("Using remote head, no local output");
        return true;
    }

    // Otherwise initialize the audio output
    HLog("Initializing audio output device");
    outputWriter = new HSoundcardWriter<int16_t>(_opts->getOutputAudioDevice(), SAMPLERATE, 1, H_SAMPLE_FORMAT_INT_16, BLOCKSIZE);

    // Output configured
    return true;
}

bool BoomaApplication::SetDumps() {

    // If we have a remote head, then do nothing
    if( _opts->getUseRemoteHead() ) {
    HLog("Using remote head, no local receiver chain");
        return true;
    }

    // Setup a splitter and a filewriter so that we can dump pcm data on request
    pcmSplitter = new HSplitter<int16_t>(processor->Consumer());
    pcmMute = new HMute<int16_t>(pcmSplitter->Consumer(), !_opts->getDumpPcm(), BLOCKSIZE);
    if( _opts->getDumpFileFormat() == WAV ) {
        pcmWriter = new HWavWriter<int16_t>("input.wav", H_SAMPLE_FORMAT_INT_16, 1, SAMPLERATE, pcmMute->Consumer());
    } else {
        pcmWriter = new HFileWriter<int16_t>("input.pcm", pcmMute->Consumer());
    }

    // Setup a splitter and a filewriter so that we can dump audio data on request
    if( _opts->getDumpFileFormat() == WAV ) {
        audioWriter = new HWavWriter<int16_t>("output.wav", H_SAMPLE_FORMAT_INT_16, 1, SAMPLERATE);
    } else {
        audioWriter = new HFileWriter<int16_t>("output.pcm");
    }
    audioMute = new HMute<int16_t>(audioWriter, !_opts->getDumpAudio(), BLOCKSIZE);
    audioSplitter = new HSplitter<int16_t>(audioMute, outputWriter);

    // Ready
    return true;
}
