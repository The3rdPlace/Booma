#include "auroralreceiver.h"

BoomaAuroralReceiver::BoomaAuroralReceiver(ConfigOptions* opts, int initialFrequency, int initialRfGain):
    BoomaReceiver(opts, initialFrequency, initialRfGain),
    _enableProbes(opts->GetEnableProbes()),
    _humfilterProbe(nullptr),
    _humfilter(nullptr) {}

HWriterConsumer<int16_t>* BoomaAuroralReceiver::PreProcess(ConfigOptions* opts, HWriterConsumer<int16_t>* previous) {
    HLog("Creating AURORAL receiver preprocessing chain");

    // Add a combfilter to kill (more) 50 hz harmonics
    HLog("Adding 50Hz humfilter for audio device input");
    _humfilterProbe = new HProbe<int16_t>("auroralreceiver_01_humfilter", _enableProbes);
    _humfilter = new HCombFilter<int16_t>(previous, opts->GetInputSampleRate(), 50, -0.907f, BLOCKSIZE, _humfilterProbe);
    return _humfilter->Consumer();
}

HWriterConsumer<int16_t>* BoomaAuroralReceiver::Receive(ConfigOptions* opts, HWriterConsumer<int16_t>* previous) {
    HLog("Creating AURORAL receiving chain");

    // Initial (fixed) gain before agc to compensate for weak input
    HLog("- RF gain");
    _gainProbe = new HProbe<int16_t>("auroralreceiver_02_gain", _enableProbes);
    _gain = new HGain<int16_t>(previous, opts->GetRfGain(), BLOCKSIZE, _gainProbe);

    // Narrow butterworth bandpass filter, bandwidth 100Hz around 1000-1100. 4th. order, 4 biquads cascaded
    HLog("- Bandpass");
    _bandpassProbe = new HProbe<int16_t>("auroralreceiver_03_bandpass", _enableProbes);
    _bandpass = new HFirFilter<int16_t>(_gain->Consumer(), HBandpassKaiserBessel<int16_t>(500, 10000, opts->GetOutputSampleRate(), 115, 96).Calculate(), 115, BLOCKSIZE, _bandpassProbe);

    // Increase signal strength before mixing to avoid losses.
    // The agc ensures that (if at all possible), the output has an average maximum amplitude of '2000'
    HLog("- RF gain");
    _agcProbe = new HProbe<int16_t>("auroralreceiver_04_agc", _enableProbes);
    _agc = new HAgc<int16_t>(_bandpass->Consumer(), 4000, 5000, 3, 20, BLOCKSIZE, _gainProbe);

    // End of receiver
    return _agc->Consumer();
}

HWriterConsumer<int16_t>* BoomaAuroralReceiver::PostProcess(ConfigOptions* opts, HWriterConsumer<int16_t>* previous) {
    HLog("Creating AURORAL receiver postprocessing chain");

    return previous;
}

BoomaAuroralReceiver::~BoomaAuroralReceiver() {
    delete _gain;
    delete _agc;
    delete _bandpass;
    delete _humfilter;

    delete _gainProbe;
    delete _agcProbe;
    delete _bandpassProbe;
    delete _humfilterProbe;
}

bool BoomaAuroralReceiver::SetFrequency(int frequency) {

    // This receiver only operates from 0 - 10KHz
    if( frequency >= 10000 || frequency < 500 ) {
        HError("Unsupported frequency %ld, must be greater than  %d and less than %d", frequency, 0, 10000);
        return false;
    }

    // Ready
    return true;
}

bool BoomaAuroralReceiver::SetRfGain(int gain) {

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