#include "auroralreceiver.h"

// Narrow butterworth bandpass filters, bandwidth 100Hz or 60hz around 1000-1100 or 770-830. 4th. order, 4 biquads cascaded
// Removes (almost) anything but the mixed down signal at the center frequency
//
// Designed using http://www.micromodeler.com/dsp/
//
// Do not forget that the a1 and a2 coefficients should be multiplied by -1 in the input coefficients
// (thus saving some cycles in the filter) - this has been done in the coefficients provided by
// micromodeler.com
//
// Narrow butterworth bandpass filter, bandwidth 100Hz around 800-900. 4th. order, 4 biquads cascaded
// Removes (almost) anything but the mixed down signal at the center frequency
//
// Designed using http://www.micromodeler.com/dsp/
float BoomaAuroralReceiver::_bandpassCoeffs[] =
{
    0.16228883979431571, 0.32457767958863143, 0.16228883979431571, 0.7171849924613378, -0.2228387332412367,// b0, b1, b2, a1, a2
    0.25, 0.5, 0.25, 0.6094520243833691, -0.5941273422731342,// b0, b1, b2, a1, a2
    1, -2, 1, 1.5282854589620225, -0.6079048053421253,// b0, b1, b2, a1, a2
    0.5, -1, 0.5, 1.821280754257546, -0.8760307482815975// b0, b1, b2, a1, a2
};
#define CENTER_FREQUENCY 835

BoomaAuroralReceiver::BoomaAuroralReceiver(ConfigOptions* opts):
    BoomaReceiver(opts),
    _enableProbes(opts->GetEnableProbes()) {}

HWriterConsumer<int16_t>* BoomaAuroralReceiver::PreProcess(ConfigOptions* opts, HWriterConsumer<int16_t>* previous) {
    HLog("Creating Auroral receiver preprocessing chain");

    // Add a passthrough block so that we can add a probe to the input
    HLog("- Passthrough (input)");
    _passthroughProbe = new HProbe<int16_t>("cwreceiver_01_input", _enableProbes);
    _passthrough = new HPassThrough<int16_t>(previous, BLOCKSIZE, _passthroughProbe);

    // Highpass filter before mixing to remove some of the lowest frequencies that may
    // get mirrored back into the final frequency range and cause (more) distortion.
    HLog("- Highpass");
    _highpassProbe = new HProbe<int16_t>("cwreceiver_02_highpass", _enableProbes);
    _highpass = new HBiQuadFilter<HHighpassBiQuad<int16_t>, int16_t>(_passthrough->Consumer(), 2000, GetSampleRate(), 0.7071f, 1, BLOCKSIZE, _highpassProbe);

    // Initial (fixed) gain before agc to compensate for weak input
    HLog("- RF gain");
    _gainProbe = new HProbe<int16_t>("cwreceiver_03_gain", _enableProbes);
    _gain = new HGain<int16_t>(_highpass->Consumer(), opts->GetRfGain(), BLOCKSIZE, _gainProbe);

    // Add a combfilter to kill (more) 50 hz harmonics
    HLog("- Humfilter");
    _humfilterProbe = new HProbe<int16_t>("cwreceiver_04_humfilter", _enableProbes);
    _humfilter = new HCombFilter<int16_t>(_gain->Consumer(), GetSampleRate(), 50, -0.707f, BLOCKSIZE, _humfilterProbe);

    // End of preprocessing
    return _humfilter->Consumer();
}

HWriterConsumer<int16_t>* BoomaAuroralReceiver::Receive(ConfigOptions* opts, HWriterConsumer<int16_t>* previous) {
    HLog("Creating CW receiver receiving chain");

    // Narrow butterworth bandpass filter, bandwidth 100Hz around 1000-1100. 4th. order, 4 biquads cascaded
    HLog("- Bandpass");
    _bandpassProbe = new HProbe<int16_t>("cwreceiver_08_bandpass", _enableProbes);
    //_bandpass = new HCascadedBiQuadFilter<int16_t>(previous, _bandpassCoeffs, 20, BLOCKSIZE, _bandpassProbe);
    _bandpass = new HBiQuadFilter<HBandpassBiQuad<int16_t>, int16_t>(previous, 5000, opts->GetSampleRate(), 0.7071f, 1, BLOCKSIZE, _bandpassProbe);

    // Increase signal strength before mixing to avoid losses.
    // The agc ensures that (if at all possible), the output has an average maximum amplitude of '2000'
    HLog("- RF gain");
    _agcProbe = new HProbe<int16_t>("cwreceiver_06_agc", _enableProbes);
    _agc = new HAgc<int16_t>(_bandpass->Consumer(), 4000, 5000, 3, 20, BLOCKSIZE, _gainProbe);

    // End of receiver
    return _agc->Consumer();
}

HWriterConsumer<int16_t>* BoomaAuroralReceiver::PostProcess(ConfigOptions* opts, HWriterConsumer<int16_t>* previous) {
    HLog("Creating CW receiver postprocessing chain");
    // No postprocessing of vlf auroral
    return previous;
}

BoomaAuroralReceiver::~BoomaAuroralReceiver() {
    delete _passthrough;
    delete _highpass;
    delete _gain;
    delete _humfilter;
    delete _agc;
    delete _bandpass;

    delete _passthroughProbe;
    delete _highpassProbe;
    delete _gainProbe;
    delete _humfilterProbe;
    delete _agcProbe;
    delete _bandpassProbe;
}

bool BoomaAuroralReceiver::SetFrequency(long int frequency) {

    // This receiver only operates from 0 - 10KHz
    if( frequency >= 10000 || frequency <= 0 ) {
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