#include "boomaamreceiver.h"

BoomaAmReceiver::BoomaAmReceiver(ConfigOptions* opts, int initialFrequency):
        BoomaReceiver(opts, initialFrequency, 3000),
        _enableProbes(opts->GetEnableProbes()),
        _absConverter(nullptr),
        _collector(nullptr),
        _gain(nullptr),
        _absConverterProbe(nullptr),
        _collectorProbe(nullptr),
        _gainProbe(nullptr) {}

HWriterConsumer<int16_t>* BoomaAmReceiver::PreProcess(ConfigOptions* opts, HWriterConsumer<int16_t>* previous) {
    HLog("Creating AM receiver preprocessing chain");

    // Add gain to adjust for the sidebands being received only having a quarter of the energy
    // as the carrier.
    HLog("Adding gain to adjust for lower signal level in the sidebands");
    _gainProbe = new HProbe<int16_t>("amreceiver_01_gain", _enableProbes);
    _gain = new HGain<int16_t>(previous, 2, BLOCKSIZE, _gainProbe);

    return _gain->Consumer();
}

HWriterConsumer<int16_t>* BoomaAmReceiver::Receive(ConfigOptions* opts, HWriterConsumer<int16_t>* previous) {
    HLog("Creating AM receiving chain");

    // The target signal is the only signal in the passband since the IQ sampler is set with center
    // frequency equal to the received center frequency, so demodulate AM from an IQ signal by
    // taking the absolute amplitude
    HLog("Demodulating AM by way of absolute value of IQ signal at time 't'");
    _absConverterProbe = new HProbe<int16_t>("amreceiver_02_abs_converter", _enableProbes);
    _absConverter = new HIq2AbsConverter<int16_t>(previous, BLOCKSIZE, _absConverterProbe);

    // Since the absolute-converter above returns only half the samples (takes a complex sample, returns
    // the magniture) we need to collect two blocks to get back to the original block size
    HLog("Collecting two blocks to reconstruct blocksize %d", BLOCKSIZE);
    _collectorProbe = new HProbe<int16_t>("amreceiver_03_collector", _enableProbes);
    _collector = new HCollector<int16_t>(_absConverter->Consumer(), BLOCKSIZE / 2, BLOCKSIZE, _collectorProbe);

    // End of receiving
    return _collector->Consumer();
}

HWriterConsumer<int16_t>* BoomaAmReceiver::PostProcess(ConfigOptions* opts, HWriterConsumer<int16_t>* previous) {
    HLog("Creating AM receiver postprocessing chain");

    return previous;
}

BoomaAmReceiver::~BoomaAmReceiver() {

    SAFE_DELETE(_absConverterProbe);
    SAFE_DELETE(_collectorProbe);
    SAFE_DELETE(_gainProbe);

    SAFE_DELETE(_absConverter);
    SAFE_DELETE(_collector);
    SAFE_DELETE(_gain);
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
