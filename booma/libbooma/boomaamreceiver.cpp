#include "boomaamreceiver.h"

BoomaAmReceiver::BoomaAmReceiver(ConfigOptions* opts, int initialFrequency):
        BoomaReceiver(opts, initialFrequency),
        _enableProbes(opts->GetEnableProbes()),
        _absConverter(nullptr),
        _collector(nullptr),
        _absConverterProbe(nullptr),
        _collectorProbe(nullptr) {}

HWriterConsumer<int16_t>* BoomaAmReceiver::PreProcess(ConfigOptions* opts, HWriterConsumer<int16_t>* previous) {
    HLog("Creating AM receiver preprocessing chain");

    _inputFirFilterProbe = new HProbe<int16_t>("amreceiver_01_inputfirfilter", _enableProbes);
    _inputFirFilter = new HIqFirFilter<int16_t>(previous, HLowpassKaiserBessel<int16_t>(8000, opts->GetOutputSampleRate(), 25, 50).Calculate(), 25, BLOCKSIZE, _inputFirFilterProbe);

    //return _gain->Consumer();
    return _inputFirFilter->Consumer();
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

    _outputFilterProbe = new HProbe<int16_t>("amreceiver_04_outputfilter", _enableProbes);
    _outputFilter = new HBiQuadFilter<HLowpassBiQuad<int16_t>, int16_t>(previous, 8000, opts->GetOutputSampleRate(), 0.707, 1, BLOCKSIZE, _outputFilterProbe);

    return _outputFilter->Consumer();
}

BoomaAmReceiver::~BoomaAmReceiver() {

    SAFE_DELETE(_absConverterProbe);
    SAFE_DELETE(_collectorProbe);

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
