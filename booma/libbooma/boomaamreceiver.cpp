#include "boomaamreceiver.h"

BoomaAmReceiver::BoomaAmReceiver(ConfigOptions* opts, int initialFrequency):
        BoomaReceiver(opts, initialFrequency, true, 3000),
        _enableProbes(opts->GetEnableProbes()),
        _absConverterProbe(nullptr),
        _collectorProbe(nullptr) {}

HWriterConsumer<int16_t>* BoomaAmReceiver::PreProcess(ConfigOptions* opts, HWriterConsumer<int16_t>* previous) {
    HLog("Creating AM receiver preprocessing chain");

    return previous;
}

HWriterConsumer<int16_t>* BoomaAmReceiver::Receive(ConfigOptions* opts, HWriterConsumer<int16_t>* previous) {
    HLog("Creating AM receiving chain");

    // The target signal is the only signal in the passband since the IQ sampler is set with center
    // frequency equal to the received center frequency, so demodulate AM from an IQ signal by
    // taking the absolute amplitude
    HLog("Demodulating AM by way of absolute value of IQ signal at time 't'");
    _absConverterProbe = new HProbe<int16_t>("amreceiver_04_abs_converter", _enableProbes);
    _absConverter = new HIq2AbsConverter<int16_t>(previous, BLOCKSIZE, _absConverterProbe);

    // Since the absolute-converter above returns only half the samples (takes a complex sample, returns
    // the magniture) we need to collect two blocks to get back to the original block size
    HLog("Collecting two blocks to reconstruct blocksize %d", BLOCKSIZE);
    _collectorProbe = new HProbe<int16_t>("amreceiver_05_collector", _enableProbes);
    _collector = new HCollector<int16_t>(_absConverter->Consumer(), BLOCKSIZE / 2, BLOCKSIZE, _collectorProbe);

    // End of receiving
    return _collector->Consumer();
}

HWriterConsumer<int16_t>* BoomaAmReceiver::PostProcess(ConfigOptions* opts, HWriterConsumer<int16_t>* previous) {
    HLog("Creating AM receiver postprocessing chain");

    return previous;
}

BoomaAmReceiver::~BoomaAmReceiver() {

    delete _absConverterProbe;
    delete _collectorProbe;

    delete _absConverter;
    delete _collector;
}

bool BoomaAmReceiver::SetFrequency(int frequency) {

    // This receiver only operates from 0 - 10KHz
    if( frequency >= 10000 || frequency <= 0 ) {
        HError("Unsupported frequency %ld, must be greater than  %d and less than %d", frequency, 0, 10000);
        return false;
    }

    // Ready
    return true;
}
