#include "internals.h"
#include <signal.h>

HProcessor<int16_t>* processor;
HReader<int16_t>* inputReader;
HWriter<int16_t>* outputWriter;

HWriter<int16_t>* pcmWriter;
HWriter<int16_t>* audioWriter;
HSplitter<int16_t>* pcmSplitter;
HSplitter<int16_t>* audioSplitter;
HMute<int16_t>* pcmMute;
HMute<int16_t>* audioMute;

BoomaReceiver* receiver;

std::thread* current;

bool terminated = false;

void setupSignalHandling()
{
    struct sigaction action;
    action.sa_handler = [](int) { terminated = true; };
    action.sa_flags = 0;
    sigemptyset (&action.sa_mask);
    sigaction (SIGINT, &action, NULL);
    sigaction (SIGTERM, &action, NULL);
}

ConfigOptions* BoomaInit(int argc, char** argv, bool verbose) {

    // Initialize the Hardt toolkit.
    // Set the last argument to 'true' to enable verbose output instead of logging to a local file
    HInit(std::string("Booma"), verbose);

    // Parse input arguments
    ConfigOptions* opts = new ConfigOptions(argc, argv);

    // Capture signals so that we can exit cleanly
    setupSignalHandling();

    // Show library name and and Hardt version.
    std::cout << "booma: using Hardt " + getversion() << std::endl;

    // Set initial receiver
    BoomaSetReceiver(opts);

    // Return the configured options
    return opts;
}

bool BoomaSetReceiver(ConfigOptions* opts) {

    // Setup input
    if( !BoomaSetInput(opts) ) {
        HError("Failed to configure input");
        return false;
    }

    // Setup output
    if( !BoomaSetOutput(opts) ) {
        HError("Failed to configure output");
        return false;
    }

    if( !BoomaSetDumps(opts) ) {
        HError("Failed to configure dumps");
        return false;
    }

    // Create receiver
    switch( opts->getReceiverModeType() ) {
        case CW:
            receiver = new BoomaCwReceiver(opts, pcmSplitter->Consumer(), audioSplitter);
            return true;
        default:
            std::cout << "Unknown receiver type defined" << std::endl;
            return false;
    }
}

bool BoomaSetInput(ConfigOptions* opts) {

    // If we are a remote head, then initialize a network processor
    if( opts->getIsRemoteHead() ) {
        HLog("Initializing network processor with remote input at %s:%d", opts->getRemoteServer(), opts->getRemotePort());
        processor = new HNetworkProcessor<int16_t>(opts->getRemoteServer(), opts->getRemotePort(), BLOCKSIZE, &terminated);
        return true;
    }

    // If we are a server for a remote head, then initialize the input and a network processor
    if( opts->getUseRemoteHead()) {
        HLog("Initializing network processor with local audio input device %d", opts->getInputAudioDevice());
        inputReader = new HSoundcardReader<int16_t>(opts->getInputAudioDevice(), SAMPLERATE, 1, H_SAMPLE_FORMAT_INT_16, BLOCKSIZE);
        processor = new HNetworkProcessor<int16_t>(opts->getRemotePort(), inputReader, BLOCKSIZE, &terminated);
        return true;
    }

    // Otherwise configure a local input
    switch( opts->getInputSourceType() ) {
        case AUDIO_DEVICE:
            HLog("Initializing audio input device %d", opts->getInputAudioDevice());
            inputReader = new HSoundcardReader<int16_t>(opts->getInputAudioDevice(), SAMPLERATE, 1, H_SAMPLE_FORMAT_INT_16, BLOCKSIZE);
            processor = new HStreamProcessor<int16_t>(inputReader, BLOCKSIZE, &terminated);
            return true;
        default:
            std::cout << "No input source type defined" << std::endl;
            return false;
    }
}

bool BoomaSetOutput(ConfigOptions* opts) {

    // If we have a remote head, then do nothing
    if( opts->getUseRemoteHead() ) {
        HLog("Using remote head, no local output");
        return true;
    }

    // Otherwise initialize the audio output
    HLog("Initializing audio output device");
    outputWriter = new HSoundcardWriter<int16_t>(opts->getOutputAudioDevice(), SAMPLERATE, 1, H_SAMPLE_FORMAT_INT_16, BLOCKSIZE);

    // Output configured
    return true;
}

bool BoomaSetDumps(ConfigOptions* opts) {

    // If we have a remote head, then do nothing
    if( opts->getUseRemoteHead() ) {
    HLog("Using remote head, no local receiver chain");
        return true;
    }

    // Setup a splitter and a filewriter so that we can dump pcm data on request
    pcmSplitter = new HSplitter<int16_t>(processor->Consumer());
    pcmMute = new HMute<int16_t>(pcmSplitter->Consumer(), !opts->getDumpPcm(), BLOCKSIZE);
    if( opts->getDumpFileFormat() == WAV ) {
        pcmWriter = new HWavWriter<int16_t>("input.wav", H_SAMPLE_FORMAT_INT_16, 1, SAMPLERATE, pcmMute->Consumer());
    } else {
        pcmWriter = new HFileWriter<int16_t>("input.pcm", pcmMute->Consumer());
    }

    // Setup a splitter and a filewriter so that we can dump audio data on request
    if( opts->getDumpFileFormat() == WAV ) {
        audioWriter = new HWavWriter<int16_t>("output.wav", H_SAMPLE_FORMAT_INT_16, 1, SAMPLERATE);
    } else {
        audioWriter = new HFileWriter<int16_t>("output.pcm");
    }
    audioMute = new HMute<int16_t>(audioWriter, !opts->getDumpAudio(), BLOCKSIZE);
    audioSplitter = new HSplitter<int16_t>(audioMute, outputWriter);

    // Ready
    return true;
}

void runner() {
    processor->Run();
}

void BoomaRun(ConfigOptions* opts) {

    // Setup the selected receiver
    HLog("Creating receiver chain for selected rx mode");
    terminated = false;

    current = new std::thread(runner);

}

void BoomaHaltReceiver() {
    terminated = true;
}