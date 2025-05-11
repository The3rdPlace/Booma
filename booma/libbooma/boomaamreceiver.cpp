#include "boomaamreceiver.h"

BoomaAmReceiver::BoomaAmReceiver(ConfigOptions* opts, int initialFrequency):
        BoomaReceiver(opts, initialFrequency),
        _absConverter(nullptr),
        _collector(nullptr) {}

HWriterConsumer<int16_t>* BoomaAmReceiver::PreProcess(ConfigOptions* opts, HWriterConsumer<int16_t>* previous) {
    HLog("Creating AM receiver preprocessing chain");

    _inputFirFilter = new HIqFirFilter<int16_t>("am_receiver_preprocess_iq_fir", previous, HLowpassKaiserBessel<int16_t>(8000, opts->GetOutputSampleRate(), 25, 50).Calculate(), 25, BLOCKSIZE);

    return _inputFirFilter->Consumer();
}

HWriterConsumer<int16_t>* BoomaAmReceiver::Receive(ConfigOptions* opts, HWriterConsumer<int16_t>* previous) {
    HLog("Creating AM receiving chain");

    // The target signal is the only signal in the passband since the IQ sampler is set with center
    // frequency equal to the received center frequency, so demodulate AM from an IQ signal by
    // taking the absolute amplitude
    HLog("Demodulating AM by way of absolute value of IQ signal at time 't'");
    _absConverter = new HIq2AbsConverter<int16_t>("am_receiver_abs_converter", previous, BLOCKSIZE);

    // Since the absolute-converter above returns only half the samples (takes a complex sample, returns
    // the magniture) we need to collect two blocks to get back to the original block size
    HLog("Collecting two blocks to reconstruct blocksize %d", BLOCKSIZE);
    _collector = new HCollector<int16_t>("am_receiver_block_collector", _absConverter->Consumer(), BLOCKSIZE / 2, BLOCKSIZE);

    // End of receiving
    return _collector->Consumer();
}

HWriterConsumer<int16_t>* BoomaAmReceiver::PostProcess(ConfigOptions* opts, HWriterConsumer<int16_t>* previous) {
    HLog("Creating AM receiver postprocessing chain");

    _outputFilter = new HBiQuadFilter<HLowpassBiQuad<int16_t>, int16_t>("am_receiver_post_process_bi_quad", previous, 3000, opts->GetOutputSampleRate(), 0.707, 1, BLOCKSIZE);

    return _outputFilter->Consumer();
}

BoomaAmReceiver::~BoomaAmReceiver() {
    SAFE_DELETE(_absConverter);
    SAFE_DELETE(_collector);
}

bool BoomaAmReceiver::SetInternalFrequency(ConfigOptions* opts, int frequency) {

    // This receiver only operates from 0 to samplerate/2.
    if( frequency >= opts->GetOutputSampleRate() / 2  ) {
        HError("Unsupported frequency %ld, must be less than %d", frequency, opts->GetOutputSampleRate() / 2);
        return false;
    }

    // Ready
    return true;
}
