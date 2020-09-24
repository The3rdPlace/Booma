#include "amreceiver.h"

BoomaAmReceiver::BoomaAmReceiver(ConfigOptions* opts, int initialFrequency, int initialRfGain):
        BoomaReceiver(opts, initialFrequency, initialRfGain),
        _enableProbes(opts->GetEnableProbes()),
        _humfilterProbe(nullptr),
        _humfilter(nullptr) {}

HWriterConsumer<int16_t>* BoomaAmReceiver::PreProcess(ConfigOptions* opts, HWriterConsumer<int16_t>* previous) {
    HLog("Creating AM receiver preprocessing chain");

    _fdec = new HIqFirDecimator<int16_t>(previous, 8, HLowpassKaiserBessel<int16_t>(5000, 1152000, 51, 50).Calculate(), 51, BLOCKSIZE);
    _dec = new HIqDecimator<int16_t>(_fdec->Consumer(), 3, BLOCKSIZE);
    _conv = new HIq2AbsConverter<int16_t>(_dec->Consumer(), BLOCKSIZE);
    _coll = new HCollector<int16_t>(_conv->Consumer(), BLOCKSIZE / 2, BLOCKSIZE);
    return _coll->Consumer();

}

HWriterConsumer<int16_t>* BoomaAmReceiver::Receive(ConfigOptions* opts, HWriterConsumer<int16_t>* previous) {
    HLog("Creating AM receiving chain");

    // Add a passthrough block so that we can add a probe to the input
    HLog("- Passthrough (input)");
    _passthroughProbe = new HProbe<int16_t>("auroralreceiver_01_input", _enableProbes);
    _passthrough = new HPassThrough<int16_t>(previous, BLOCKSIZE, _passthroughProbe);

    // Highpass filter before mixing to remove some of the lowest frequencies that may
    // get mirrored back into the final frequency range and cause (more) distortion.
    HLog("- Highpass");
    _highpassProbe = new HProbe<int16_t>("auroralreceiver_02_highpass", _enableProbes);
    _highpass = new HBiQuadFilter<HHighpassBiQuad<int16_t>, int16_t>(_passthrough->Consumer(), 500, opts->GetOutputSampleRate(), 0.7071f, 1, BLOCKSIZE, _highpassProbe);

    // Initial (fixed) gain before agc to compensate for weak input
    HLog("- RF gain");
    _gainProbe = new HProbe<int16_t>("auroralreceiver_03_gain", _enableProbes);
    _gain = new HGain<int16_t>(_highpass->Consumer(), opts->GetRfGain(), BLOCKSIZE, _gainProbe);

    // Narrow butterworth bandpass filter, bandwidth 100Hz around 1000-1100. 4th. order, 4 biquads cascaded
    HLog("- Bandpass");
    _bandpassProbe = new HProbe<int16_t>("auroralreceiver_04_bandpass", _enableProbes);
    //_bandpass = new HCascadedBiQuadFilter<int16_t>(_gain->Consumer(), _bandpassCoeffs, 20, BLOCKSIZE, _bandpassProbe);
    _bandpass = new HFirFilter<int16_t>(_gain->Consumer(), HBandpassKaiserBessel<int16_t>(500, 10000, opts->GetOutputSampleRate(), 115, 50).Calculate(), 115, BLOCKSIZE, _bandpassProbe);

    // Increase signal strength before mixing to avoid losses.
    // The agc ensures that (if at all possible), the output has an average maximum amplitude of '2000'
    HLog("- RF gain");
    _agcProbe = new HProbe<int16_t>("auroralreceiver_05_agc", _enableProbes);
    _agc = new HAgc<int16_t>(_bandpass->Consumer(), 4000, 5000, 3, 20, BLOCKSIZE, _gainProbe);

    // End of receiver
    return _agc->Consumer();
}

HWriterConsumer<int16_t>* BoomaAmReceiver::PostProcess(ConfigOptions* opts, HWriterConsumer<int16_t>* previous) {
    HLog("Creating AM receiver postprocessing chain");

    return previous;
}

BoomaAmReceiver::~BoomaAmReceiver() {
    delete _passthrough;
    delete _highpass;
    delete _gain;
    delete _agc;
    delete _bandpass;

    delete _passthroughProbe;
    delete _highpassProbe;
    delete _gainProbe;
    delete _agcProbe;
    delete _bandpassProbe;

    if( _humfilter != nullptr ) {
        delete _humfilter;
    }
    if( _humfilterProbe != nullptr ) {
        delete _humfilterProbe;
    }
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

bool BoomaAmReceiver::SetRfGain(int gain) {

    // Sane gain values are between 1 and 100
    if( gain < 1 || gain > 50 ) {
        HError("Unsupported rf gain value %d, must be between 1 and 50", gain);
        return false;
    }

    // Set rf gain
    _gain->SetGain(gain);

    // Done
    return true;
}