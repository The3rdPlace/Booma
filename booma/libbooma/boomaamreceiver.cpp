#include "boomaamreceiver.h"

BoomaAmReceiver::BoomaAmReceiver(ConfigOptions* opts, int initialFrequency):
        BoomaReceiver(opts, initialFrequency),
        _enableProbes(opts->GetEnableProbes()),
        _humfilterProbe(nullptr) {}

HWriterConsumer<int16_t>* BoomaAmReceiver::PreProcess(ConfigOptions* opts, HWriterConsumer<int16_t>* previous) {
    HLog("Creating AM receiver preprocessing chain");

    // Fir-decimator to filter a 3Khz passband and bring the samplerate down 8 times
    HLog("Creating FIR decimator (%d)", 8);
    _firDecimator = new HIqFirDecimator<int16_t>(previous, 8, HLowpassKaiserBessel<int16_t>( 3000, 1152000, 15, 96).Calculate(), 15, BLOCKSIZE);

    // Extra Fir filter to remove signals outside of the 3Khz passband
    HLog("Creating passband FIR filter");
    _firFilter = new HIqFirFilter<int16_t>(_firDecimator->Consumer(), HLowpassKaiserBessel<int16_t>( 3000, 144000, 15, 96).Calculate(), 15, BLOCKSIZE);

    // Decimate down to the output rate
    HLog("Creating decimator to bring samplerate down the output rate");
    _decimator = new HIqDecimator<int16_t>(_firFilter->Consumer(), 3, BLOCKSIZE);

    // End of preprocesssing
    return _decimator->Consumer();
}

HWriterConsumer<int16_t>* BoomaAmReceiver::Receive(ConfigOptions* opts, HWriterConsumer<int16_t>* previous) {
    HLog("Creating AM receiving chain");

    // The target signal is now the only signal in the passband, so demodulate AM
    // from an IQ signal by taking the absolute amplitude
    HLog("Demodulating AM by way of absolute value of IQ signal at time 't'");
    _absConverter = new HIq2AbsConverter<int16_t>(previous, BLOCKSIZE);

    // Since the absolute-converter above returns only half the samples (takes a complex sample, returns
    // the magniture) we need to collect two blocks to get back to the original block size
    HLog("Collecting two blocks to reconstruct blocksize %d", BLOCKSIZE);
    _collector = new HCollector<int16_t>(_absConverter->Consumer(), BLOCKSIZE / 2, BLOCKSIZE);

    // End of receiving
    return _collector->Consumer();
}

HWriterConsumer<int16_t>* BoomaAmReceiver::PostProcess(ConfigOptions* opts, HWriterConsumer<int16_t>* previous) {
    HLog("Creating AM receiver postprocessing chain");

    return previous;
}

BoomaAmReceiver::~BoomaAmReceiver() {

    delete _firDecimator;
    delete _firFilter;
    delete _decimator;
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
