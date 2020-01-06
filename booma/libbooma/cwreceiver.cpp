#include "cwreceiver.h"

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
float BoomaCwReceiver::_bandpassCoeffs[] =
{
    0.06318758096656742, -0.12637516193313483, 0.06318758096656742, 1.9822798942156608, -0.9928732078139422,// b0, b1, b2, a1, a2
    0.0625, -0.125, 0.0625, 1.986908612864604, -0.9971024098670853,// b0, b1, b2, a1, a2
    0.000244140625, 0.00048828125, 0.000244140625, 1.9814486970269125, -0.9926669567470747,// b0, b1, b2, a1, a2
    0.000244140625, 0.00048828125, 0.000244140625, 1.9851881609275535, -0.996895491657892// b0, b1, b2, a1, a2
};
#define CENTER_FREQUENCY 835

BoomaCwReceiver::BoomaCwReceiver(ConfigOptions* opts, BoomaInput* input): //  int frequency, int gain,
    BoomaReceiver(opts),
    _enableProbes(opts->GetEnableProbes()) {
    HLog("Creating CW receiver chain");

    // Add a passthrough block so that we can add a probe to the input
    HLog("- Passthrough (input)");
    _passthroughProbe = new HProbe<int16_t>("cwreceiver_01_input", _enableProbes);
    _passthrough = new HPassThrough<int16_t>(input->GetLastConsumer(), BLOCKSIZE, _passthroughProbe);

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
    _humfilter = new HCombFilter<int16_t>(_gain->Consumer(), GetSampleRate(), 50, -0.907f, BLOCKSIZE, _humfilterProbe);

    // Add a splitter so that the outside world can (if requested) add a spectrum (or other) writer */
    _spectrum = new HSplitter<int16_t>(_humfilter->Consumer());

    // Bandpass filter before mixing to remove or reduce frequencies we do not want to mix
    HLog("- Preselect");
    _preselectProbe = new HProbe<int16_t>("cwreceiver_05_preselect", _enableProbes);
    _preselect = new HBiQuadFilter<HBandpassBiQuad<int16_t>, int16_t>(_spectrum->Consumer(), opts->GetFrequency(), GetSampleRate(), 0.7071f, 1, BLOCKSIZE, _preselectProbe);

    // Increase signal strength before mixing to avoid losses.
    // The agc ensures that (if at all possible), the output has an average maximum amplitude of '2000'
    HLog("- RF gain");
    _agcProbe = new HProbe<int16_t>("cwreceiver_06_agc", _enableProbes);
    _agc = new HAgc<int16_t>(_preselect->Consumer(), 2000, 2500, 3, BLOCKSIZE, _gainProbe);

    // Mix down to the output frequency.
    // 17200Hz - 16360Hz = 840Hz  (place it somewhere inside the bandpass filter pass region)
    // The multiplier localoscillator runs with a maximum amplitude of '10' which ensures that we
    // should never have any overflows when combined with the output of the previous AGC since 10 * 2000 = 20.000
    HLog("- Mixer");
    _multiplierProbe = new HProbe<int16_t>("cwreceiver_07_multiplier", _enableProbes);
    _multiplier = new HMultiplier<int16_t>(_agc->Consumer(), GetSampleRate(), opts->GetFrequency() - CENTER_FREQUENCY, 10, BLOCKSIZE, _multiplierProbe);

    // Narrow butterworth bandpass filter, bandwidth 100Hz around 1000-1100. 4th. order, 4 biquads cascaded
    HLog("- Bandpass");
    _bandpassProbe = new HProbe<int16_t>("cwreceiver_08_bandpass", _enableProbes);
    _bandpass = new HCascadedBiQuadFilter<int16_t>(_multiplier->Consumer(), _bandpassCoeffs, 20, BLOCKSIZE, _bandpassProbe);

    // Smoother bandpass filter (2 stacked biquads) to remove artifacts from the very narrow detector
    // filter above
    _postSelectProbe = new HProbe<int16_t>("cwreceiver_09_postselect", _enableProbes);
    _postSelect = new HCascadedBiQuadFilter<int16_t>(_bandpass->Consumer(), {
        HBandpassBiQuad<int16_t>(CENTER_FREQUENCY + 50, GetSampleRate(), 0.207f, 1).Calculate(),
        HBandpassBiQuad<int16_t>(CENTER_FREQUENCY - 50, GetSampleRate(), 0.207f, 1).Calculate()
    }, BLOCKSIZE, _postSelectProbe);
}

BoomaCwReceiver::~BoomaCwReceiver() {
    delete _passthrough;
    delete _highpass;
    delete _gain;
    delete _humfilter;
    delete _spectrum;
    delete _preselect;
    delete _agc;
    delete _multiplier;
    delete _bandpass;
    delete _postSelect;

    delete _passthroughProbe;
    delete _highpassProbe;
    delete _gainProbe;
    delete _humfilterProbe;
    delete _preselectProbe;
    delete _agcProbe;
    delete _multiplierProbe;
    delete _bandpassProbe;
    delete _postSelectProbe;
}

bool BoomaCwReceiver::SetFrequency(long int frequency) {

    // This receiver only operates from 820 - samplerate/2
    if( frequency >= GetSampleRate() / 2 || frequency <= CENTER_FREQUENCY ) {
        HError("Unsupported frequency %ld, must be greater than  %d and less than %d", frequency, CENTER_FREQUENCY, GetSampleRate() / 2);
        return false;
    }

    // Set new multiplier frequency and adjust the preselect bandpass filter
    _multiplier->SetFrequency(frequency - CENTER_FREQUENCY);
    ((HBiQuadFilter<HBandpassBiQuad<int16_t>, int16_t>*) _preselect)->SetCoefficients(frequency, GetSampleRate(), 0.7071f, 1, BLOCKSIZE);

    // Ready
    return true;
}

bool BoomaCwReceiver::SetRfGain(int gain) {

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