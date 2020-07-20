#include "cwreceiver2.h"

float BoomaCwReceiver2::_bandpassCoeffs[3][20] =
{
    // 50 Hz @ 6KHz
    {
        0.002048572294543357, 0.004097144589086714, 0.002048572294543357, 1.408067592516551, -0.9938554560160351,// b0, b1, b2, a1, a2
        0.0078125, -0.015625, 0.0078125, 1.4116734879341142, -0.9938710552646586,// b0, b1, b2, a1, a2
        0.0009765625, 0.001953125, 0.0009765625, 1.408045459371766, -0.9974456925740307,// b0, b1, b2, a1, a2
        0.0078125, -0.015625, 0.0078125, 1.4167508604593735, -0.9974613476810194// b0, b1, b2, a1, a2
    },

    // 100 Hz @ 6KHz
    {
        0.0033509644355716028, 0.0067019288711432055, 0.0033509644355716028, 1.4026434633408733, -0.9883093940592698,// b0, b1, b2, a1, a2
        0.015625, -0.03125, 0.015625, 1.4095146965221208, -0.9883657437885459,// b0, b1, b2, a1, a2
        0.001953125, 0.00390625, 0.001953125, 1.4025641509285973, -0.9951244773940543,// b0, b1, b2, a1, a2
        0.015625, -0.03125, 0.015625, 1.4191527482568658, -0.9951812092261056// b0, b1, b2, a1, a2
    },

    // 200 Hz @ 6KHz
    {
        0.007439398273347204, 0.014878796546694408, 0.007439398273347204, 1.3903155004925956, -0.9759828267704111,// b0, b1, b2, a1, a2
        0.03125, -0.0625, 0.03125, 1.4044836752325582, -0.9762194821504804,// b0, b1, b2, a1, a2
        0.00390625, 0.0078125, 0.00390625, 1.3899786395895477, -0.9899122406330527,// b0, b1, b2, a1, a2
        0.03125, -0.0625, 0.03125, 1.4241834185016982, -0.9901521572645293// b0, b1, b2, a1, a2
    }
};

int BoomaCwReceiver2::_bandpassWidths[] =
{
    50,
    100,
    200
};

float BoomaCwReceiver2::_cwCoeffs[] =
{
    // 400 Hz @ 1000 Hz
    0.001780904520508367, 0.003561809041016734, 0.001780904520508367, 1.9305185311207878, -0.9492636159174858,// b0, b1, b2, a1, a2
    0.5, -1, 0.5, 1.9427984657253758, -0.9565053806179769,// b0, b1, b2, a1, a2
    0.001953125, 0.00390625, 0.001953125, 1.953016385053386, -0.9766112182691846,// b0, b1, b2, a1, a2
    0.25, -0.5, 0.25, 1.9726460120356664, -0.9838372084850454// b0, b1, b2, a1, a2
};

BoomaCwReceiver2::BoomaCwReceiver2(ConfigOptions* opts):
    BoomaReceiver(opts),
    _enableProbes(opts->GetEnableProbes()) {

        std::vector<OptionValue> bandwidthValues {
            OptionValue {"Narrow", "Narrow IF filter at 50Hz", 0},
            OptionValue {"Medium", "Medium IF filter at 100Hz", 1},
            OptionValue {"Wide", "Wide IF filter at 200Hz", 2}};
        std::vector<OptionValue> beattoneValues {
            OptionValue {"800", "CW Beattone 800Hz", 800},
            OptionValue {"840", "CW Beattone 840Hz", 840},
            OptionValue {"880", "CW Beattone 880Hz", 880},
            OptionValue {"920", "CW Beattone 920Hz", 920},
            OptionValue {"960", "CW Beattone 960Hz", 960},
            OptionValue {"1000", "CW Beattone 1000Hz", 1000},
            OptionValue {"1040", "CW Beattone 1040Hz", 1040},
            OptionValue {"1080", "CW Beattone 1080Hz", 1080},
            OptionValue {"1120", "CW Beattone 1120Hz", 1120},
            OptionValue {"1160", "CW Beattone 1160Hz", 1160},
            OptionValue {"1200", "CW Beattone 1200Hz", 1200}};
        std::vector<OptionValue> ifshiftValues {
            OptionValue {"Left", "Shift IF passband to the left", -1},
            OptionValue {"Center", "Center IF passband", 0},
            OptionValue {"Right", "Shift IF passband to the right", 1}};

        Option bandwidthOption {
            "Bandwidth",
            "Bandwidth of IF filter",
            bandwidthValues,
            1
        };
        Option beattoneOption {
                "Beattone",
                "CW beattone frequency",
                beattoneValues,
                840
        };
        Option ifshiftOption {
                "Ifshift",
                "IF shift",
                ifshiftValues,
                0
        };

        // Register options
        RegisterOption(bandwidthOption);
        RegisterOption(beattoneOption);
        RegisterOption(ifshiftOption);
    }

HWriterConsumer<int16_t>* BoomaCwReceiver2::PreProcess(ConfigOptions* opts, HWriterConsumer<int16_t>* previous) {
    HLog("Creating CW2 receiver preprocessing chain");

    // Add a passthrough block so that we can add a probe to the input
    HLog("- Passthrough (input)");
    _passthroughProbe = new HProbe<int16_t>("cwreceiver2_01_input", _enableProbes);
    _passthrough = new HPassThrough<int16_t>(previous, BLOCKSIZE, _passthroughProbe);

    // Band filter before mixing to remove some of the lowest frequencies that may
    // get mirrored back into the final frequency range and cause (more) distortion.
    HLog("- Bandpass");
    _highpassProbe = new HProbe<int16_t>("cwreceiver2_02_highpass", _enableProbes);
    _highpass = new HBiQuadFilter<HHighpassBiQuad<int16_t>, int16_t>(_passthrough->Consumer(), 2000, GetSampleRate(), 0.7071f, 1, BLOCKSIZE, _highpassProbe);

    // Initial (fixed) gain before agc to compensate for weak input
    HLog("- RF gain");
    _gainProbe = new HProbe<int16_t>("cwreceiver2_03_gain", _enableProbes);
    _gain = new HGain<int16_t>(_highpass->Consumer(), opts->GetRfGain(), BLOCKSIZE, _gainProbe);

    // Add a combfilter to kill (more) 50 hz harmonics
    HLog("- Humfilter");
    _humfilterProbe = new HProbe<int16_t>("cwreceiver2_04_humfilter", _enableProbes);
    _humfilter = new HCombFilter<int16_t>(_gain->Consumer(), GetSampleRate(), 50, -0.707f, BLOCKSIZE, _humfilterProbe);

    // End of preprocessing
    return _humfilter->Consumer();
}

HWriterConsumer<int16_t>* BoomaCwReceiver2::Receive(ConfigOptions* opts, HWriterConsumer<int16_t>* previous) {
    HLog("Creating CW2 receiver receiving chain");

    // Calculate mixer offsets due to if filter shifting
    int offset = GetOption("Ifshift") * (_bandpassWidths[GetOption("Bandwidth")] / 4);
    HLog("IF mixer offset due to IF shift set to %dHz", offset);

    // Bandpass filter before mixing to remove or reduce frequencies we do not want to mix
    HLog("- Preselect");
    _preselectProbe = new HProbe<int16_t>("cwreceiver2_05_preselect", _enableProbes);
    _preselect = new HBiQuadFilter<HBandpassBiQuad<int16_t>, int16_t>(previous, opts->GetFrequency() + offset, GetSampleRate(), 1.0f, 1, BLOCKSIZE, _preselectProbe);

    // Increase signal strength before mixing to avoid losses.
    // The agc ensures that (if at all possible), the output has an average maximum amplitude of '2000'
    HLog("- RF gain");
    _agcProbe = new HProbe<int16_t>("cwreceiver2_06_agc", _enableProbes);
    _agc = new HAgc<int16_t>(_preselect->Consumer(), 2000, 3000, 3, 20, BLOCKSIZE, _gainProbe);

    // Mix down to IF frequency = 6000Hz
    HLog("- IF Mixer");
    _ifMixerProbe = new HProbe<int16_t>("cwreceiver2_07_if_mixer", _enableProbes);
    _ifMixer = new HMultiplier<int16_t>(_agc->Consumer(), GetSampleRate(), opts->GetFrequency() - 6000 + offset, 10, BLOCKSIZE, _ifMixerProbe);

    // Narrow if filter consisting of a number of cascaded 2. order bandpass filters
    HLog("- IF filter");
    _ifFilterProbe = new HProbe<int16_t>("cwreceiver_08_if_filter", _enableProbes);
    _ifFilter = new HCascadedBiQuadFilter<int16_t>(_ifMixer->Consumer(), _bandpassCoeffs[GetOption("Bandwidth")], 20, BLOCKSIZE, _ifFilterProbe);

    // Mix down to the output frequency.
    // 6000Hz - 5160Hz = 840Hz
    HLog("- Beat tone mixer");
    _beatToneMixerProbe = new HProbe<int16_t>("cwreceiver2_09_beat_tone_mixer", _enableProbes);
    _beatToneMixer = new HMultiplier<int16_t>(_ifFilter->Consumer(), GetSampleRate(), 6000 - GetOption("Beattone") - offset, 1, BLOCKSIZE, _beatToneMixerProbe);

    // Smoother bandpass filter (2 stacked biquads) to remove artifacts from the very narrow detector
    // filter above
    HLog("- Output filter");
    _postSelectProbe = new HProbe<int16_t>("cwreceiver2_10_postselect", _enableProbes);
    _postSelect = new HCascadedBiQuadFilter<int16_t>(_beatToneMixer->Consumer(), _cwCoeffs, 20, BLOCKSIZE, _postSelectProbe);

    // End of receiver
    return _postSelect->Consumer();
}

HWriterConsumer<int16_t>* BoomaCwReceiver2::PostProcess(ConfigOptions* opts, HWriterConsumer<int16_t>* previous) {
    HLog("Creating CW2 receiver postprocessing chain");
    // No postprocessing of vlf cw
    return previous;
}

BoomaCwReceiver2::~BoomaCwReceiver2() {
    delete _passthrough;
    delete _highpass;
    delete _gain;
    delete _humfilter;
    delete _preselect;
    delete _agc;
    delete _ifMixer;
    delete _ifFilter;
    delete _beatToneMixer;
    delete _postSelect;
    
    delete _passthroughProbe;
    delete _highpassProbe;
    delete _gainProbe;
    delete _humfilterProbe;
    delete _preselectProbe;
    delete _agcProbe;
    delete _ifMixerProbe;
    delete _ifFilterProbe;
    delete _beatToneMixerProbe;
    delete _postSelectProbe;
}

bool BoomaCwReceiver2::SetFrequency(long int frequency) {

    // This receiver only operates from 6000 - samplerate/2
    if( frequency >= GetSampleRate() / 2 || frequency <= 6000 ) {
        HError("Unsupported frequency %ld, must be greater than  %d and less than %d", frequency, 6000, GetSampleRate() / 2);
        return false;
    }

    // Set new multiplier frequency and adjust the preselect bandpass filter
    _ifMixer->SetFrequency(frequency - 6000);

    // Ready
    return true;
}

bool BoomaCwReceiver2::SetRfGain(int gain) {

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

void BoomaCwReceiver2::OptionChanged(ConfigOptions* opts, std::string name, int value) {
    HLog("Option %s has changed to value %d", name.c_str(), value);

    // Calculate mixer offsets due to if filter shifting
    int offset = GetOption("Ifshift") * (_bandpassWidths[GetOption("Bandwidth")] / 4);
    HLog("IF mixer offset due to IF shift set to %dHz", offset);

    // Reconfigure receiver
    _preselect->SetCoefficients(opts->GetFrequency() + offset, GetSampleRate(), 1.0f, 1, BLOCKSIZE); 
    _ifMixer->SetFrequency(opts->GetFrequency() - 6000 + offset);
    _ifFilter->SetCoefficients(_bandpassCoeffs[GetOption("Bandwidth")], 20);
    _beatToneMixer->SetFrequency(6000 - GetOption("Beattone") - offset);
    HLog("Receiver chain reconfigured");
}

std::string BoomaCwReceiver2::GetOptionInfoString() { 
    std::string info = "bw:" + std::to_string(_bandpassWidths[GetOption("Bandwidth")]) + "Hz";
    if( GetOption("Ifshift") != 0 ) {
        int shift = _bandpassWidths[GetOption("Bandwidth")] / 4;
        info += (GetOption("Ifshift") > 0 ? " " + std::to_string(shift) + "-->" : " <--" + std::to_string(shift));
    } 
    return  info;
}