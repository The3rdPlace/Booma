#include "boomaauroralreceiver.h"

BoomaAuroralReceiver::BoomaAuroralReceiver(ConfigOptions* opts, int initialFrequency):
    BoomaReceiver(opts, initialFrequency),
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

    // Narrow butterworth bandpass filter, bandwidth 100Hz around 1000-1100. 4th. order, 4 biquads cascaded
    HLog("- Bandpass");
    _bandpassProbe = new HProbe<int16_t>("auroralreceiver_02_bandpass", _enableProbes);
    _bandpass = new HFirFilter<int16_t>(previous, HBandpassKaiserBessel<int16_t>(500, 10000, opts->GetOutputSampleRate(), 115, 96).Calculate(), 115, BLOCKSIZE, _bandpassProbe);

    // End of receiver
    return _bandpass->Consumer();
}

HWriterConsumer<int16_t>* BoomaAuroralReceiver::PostProcess(ConfigOptions* opts, HWriterConsumer<int16_t>* previous) {
    HLog("Creating AURORAL receiver postprocessing chain");

    return previous;
}

BoomaAuroralReceiver::~BoomaAuroralReceiver() {
    SAFE_DELETE(_bandpass);
    SAFE_DELETE(_humfilter);

    SAFE_DELETE(_bandpassProbe);
    SAFE_DELETE(_humfilterProbe);
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
