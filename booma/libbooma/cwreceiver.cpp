#include "cwreceiver.h"

// Narrow butterworth bandpass filters, bandwidth 100Hz or 60hz around 1000-1100 or 770-830. 4th. order, 4 biquads cascaded
// Removes (almost) anything but the mixed down signal at the center frequency
//
// Designed using http://www.micromodeler.com/dsp/
//
// Do not forget that the a1 and a2 coefficients should be multiplied by -1 in the input coefficients
// (thus saving some cycles in the filter)
// Narrow butterworth bandpass filter, bandwidth 100Hz around 1000-1100. 4th. order, 4 biquads cascaded
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
#define CW_TONE_FREQUENCY 840

BoomaCwReceiver::BoomaCwReceiver(int samplerate, int frequency, int gain, HWriterConsumer<int16_t>* previous, bool enableProbes):
    BoomaReceiver(samplerate),
    _enableProbes(enableProbes) {
    HLog("Creating CW receiver chain");

    // Add a passthrough block so that we can add a probe to the input
    HLog("- Passthrough (input)");
    _passthroughProbe = new HProbe<int16_t>("cwreceiver_01_input", _enableProbes);
    _passthrough = new HPassThrough<int16_t>(previous, BLOCKSIZE, _passthroughProbe);

    // Add a combfilter to kill (more) 50 hz harmonics
    HLog("- Humfilter");
    _humfilterProbe = new HProbe<int16_t>("cwreceiver_02_humfilter", _enableProbes);
    _humfilter = new HCombFilter<int16_t>(_passthrough->Consumer(), GetSampleRate(), 50, -0.907f, BLOCKSIZE, _humfilterProbe);

    // Increase signal strength after mixing to avoid losses before filtering and mixing
    HLog("- RF gain");
    _gainProbe = new HProbe<int16_t>("cwreceiver_03_gain", _enableProbes);
    _gain = new HGain<int16_t>(_humfilter->Consumer(), gain, BLOCKSIZE, _gainProbe);

    // Highpass filter before mixing to remove some of the lowest frequencies that may
    // get mirrored back into the final frequency range and cause (more) distortion.
    // (In this receiver, the results are good when the cutoff frequency is located at the local oscillator frequency)
    HLog("- Preselect");
    _preselectProbe = new HProbe<int16_t>("cwreceiver_04_preselect", _enableProbes);
    _preselect = new HBiQuadFilter<HBandpassBiQuad<int16_t>, int16_t>(_gain->Consumer(), frequency, GetSampleRate(), 0.7071f, 1, BLOCKSIZE, _preselectProbe);

    // Mix down to the output frequency.
    // 17200Hz - 16160Hz = 1040Hz  (place it somewhere inside the bandpass filter pass region)
    HLog("- Mixer");
    _multiplierProbe = new HProbe<int16_t>("cwreceiver_05_multiplier", _enableProbes);
    _multiplier = new HMultiplier<int16_t>(_preselect->Consumer(), GetSampleRate(), frequency - CW_TONE_FREQUENCY, BLOCKSIZE, _multiplierProbe);

    // Narrow butterworth bandpass filter, bandwidth 100Hz around 1000-1100. 4th. order, 4 biquads cascaded
    HLog("- Bandpass");
    _bandpassProbe = new HProbe<int16_t>("cwreceiver_06_bandpass", _enableProbes);
    _bandpass = new HCascadedBiQuadFilter<int16_t>(_multiplier->Consumer(), _bandpassCoeffs, 20, BLOCKSIZE, _bandpassProbe);

    // Smoother bandpass filter (2 stacked biquads) to remove artifacts from the very narrow detector
    // filter above
    _postSelectProbe = new HProbe<int16_t>("cwreceiver_07_postselect", _enableProbes);
    _postSelect = new HCascadedBiQuadFilter<int16_t>(_bandpass->Consumer(), {
        HBandpassBiQuad<int16_t>(CW_TONE_FREQUENCY, GetSampleRate(), 0.307f, 1).Calculate(),
        HBandpassBiQuad<int16_t>(CW_TONE_FREQUENCY, GetSampleRate(), 0.307f, 1).Calculate()
    }, BLOCKSIZE, _postSelectProbe);
}

BoomaCwReceiver::~BoomaCwReceiver() {
    delete _gain;
    delete _preselect;
    delete _multiplier;

    delete _bandpass;

    delete _postSelect;

    delete _passthrough;
    delete _passthroughProbe;
    delete _gainProbe;
    delete _preselectProbe;
    delete _multiplierProbe;
    delete _bandpassProbe;
    delete _postSelectProbe;
}

bool BoomaCwReceiver::SetFrequency(long int frequency) {

    // This receiver only operates from 820 - samplerate/2
    if( frequency >= GetSampleRate() / 2 || frequency <= CW_TONE_FREQUENCY ) {
        HError("Unsupported frequency %ld, must be greater than  %d and less than %d", frequency, CW_TONE_FREQUENCY, GetSampleRate() / 2);
        return false;
    }

    // Set new multiplier frequency and adjust the preselect bandpass filter
    _multiplier->SetFrequency(frequency - CW_TONE_FREQUENCY);
    ((HBiQuadFilter<HBandpassBiQuad<int16_t>, int16_t>*) _preselect)->SetCoefficients(frequency, GetSampleRate(), 0.8071f, 1, BLOCKSIZE);

    // Ready
    return true;
}

bool BoomaCwReceiver::SetRfGain(int gain) {

    // Sane gain values are between 1 and 100
    if( gain < 1 || gain > 100 ) {
        HError("Unsupported rf gain value %d, must be between 1 and 100", gain);
        return false;
    }

    // Set rf gain
    _gain->SetGain(gain);

    // Done
    return true;
}