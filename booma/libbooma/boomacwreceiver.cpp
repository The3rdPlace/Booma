#include "boomacwreceiver.h"

float BoomaCwReceiver::_bandpassCoeffs[6][20] =
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
    },

    // 500 Hz @ 6KHz
    {
        0.01472079702256931, 0.02944159404513862, 0.01472079702256931, 1.3578013970831393, -0.9429178695624337,// b0, b1, b2, a1, a2
        0.125, -0.25, 0.125, 1.3918033585781613, -0.9442389046006642,// b0, b1, b2, a1, a2
        0.0078125, 0.015625, 0.0078125, 1.3559075775374545, -0.9755717421203991,// b0, b1, b2, a1, a2
        0.0625, -0.125, 0.0625, 1.437988425774804, -0.9769347083141211// b0, b1, b2, a1, a2
    },

    // 1  KHz @ 6KHz
    {
        0.03309018753561541, 0.06618037507123083, 0.03309018753561541, 1.2999716556158074, -0.8820266316104814,// b0, b1, b2, a1, a2
        0.25, -0.5, 0.25, 1.3714944005456748, -0.8875762227807565,// b0, b1, b2, a1, a2
        0.015625, 0.03125, 0.015625, 1.2920291704550972, -0.9478234710900391,// b0, b1, b2, a1, a2
        0.125, -0.25, 0.125, 1.46455173397263, -0.9537131436046499// b0, b1, b2, a1, a2
    },

    // 3 KHz @ 6KHz
    {
            0.1194878224582199, 0.2389756449164398, 0.1194878224582199, 1.1052801524695883, -0.670102919351549,// b0, b1, b2, a1, a2
            0.5, -1, 0.5, 1.3208341719229677, -0.712714773030205,// b0, b1, b2, a1, a2
            0.0625, 0.125, 0.0625, 1.0466041855109196, -0.8394505074497361,// b0, b1, b2, a1, a2
            0.25, -0.5, 0.25, 1.5573059390887027, -0.8869145132418825// b0, b1, b2, a1, a2
    }
};

int BoomaCwReceiver::_bandpassWidths[] =
{
    50,
    100,
    200,
    500,
    1000,
    3000
};

float BoomaCwReceiver::_cwCoeffs[] =
{
    // 400 Hz @ 1000 Hz
    0.001780904520508367, 0.003561809041016734, 0.001780904520508367, 1.9305185311207878, -0.9492636159174858,// b0, b1, b2, a1, a2
    0.5, -1, 0.5, 1.9427984657253758, -0.9565053806179769,// b0, b1, b2, a1, a2
    0.001953125, 0.00390625, 0.001953125, 1.953016385053386, -0.9766112182691846,// b0, b1, b2, a1, a2
    0.25, -0.5, 0.25, 1.9726460120356664, -0.9838372084850454// b0, b1, b2, a1, a2
};

BoomaCwReceiver::BoomaCwReceiver(ConfigOptions* opts, int initialFrequency):
        BoomaReceiver(opts, initialFrequency),
        _enableProbes(opts->GetEnableProbes()),
        _humfilterProbe(nullptr),
        _iq2IConverterProbe(nullptr),
        _iqMultiplierProbe(nullptr),
        _preselectProbe(nullptr),
        _ifMixerProbe(nullptr),
        _ifFilterProbe(nullptr),
        _beatToneMixerProbe(nullptr),
        _postSelectProbe(nullptr),
        _humfilter(nullptr),
        _iq2IConverter(nullptr),
        _iqMultiplier(nullptr),
        _preselect(nullptr),
        _ifMixer(nullptr),
        _ifFilter(nullptr),
        _beatToneMixer(nullptr),
        _postSelect(nullptr),
        _passbandGain(nullptr),
        _passbandGainProbe(nullptr) {

        std::vector<OptionValue> bandwidthValues {
            OptionValue {"50", "Narrow CW IF filter at 50Hz", 0},
            OptionValue {"100", "Medium CW IF filter at 100Hz", 1},
            OptionValue {"200", "Wide CW IF filter at 200Hz", 2},
            OptionValue {"500", "Narrow SSB(CW) IF filter at 500Hz", 3},
            OptionValue {"1000", "Medium SSB(CW) IF filter at 1KHz", 4},
            OptionValue {"3000", "Wide SSB(CW) IF filter at 3KHz", 5}};
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
        std::vector<OptionValue> passbandGainValues {
                OptionValue {"0", "Passband gain factor 1", 1},
                OptionValue {"1", "Passband gain factor 10", 10},
                OptionValue {"2", "Passband gain factor 2", 20},
                OptionValue {"3", "Passband gain factor 3", 30},
                OptionValue {"4", "Passband gain factor 4", 40},
                OptionValue {"5", "Passband gain factor 5", 50},
                OptionValue {"6", "Passband gain factor 6", 60}};
        std::vector<OptionValue> iqPassbandGainValues {
                OptionValue {"0", "Passband gain factor 0.5", 0},
                OptionValue {"1", "Passband gain factor 1", 1},
                OptionValue {"2", "Passband gain factor 2", 2},
                OptionValue {"3", "Passband gain factor 3", 3},
                OptionValue {"4", "Passband gain factor 4", 4},
                OptionValue {"5", "Passband gain factor 5", 5},
                OptionValue {"6", "Passband gain factor 6", 6},
                OptionValue {"7", "Passband gain factor 6", 7},
                OptionValue {"8", "Passband gain factor 6", 8},
                OptionValue {"9", "Passband gain factor 6", 9},
                OptionValue {"10", "Passband gain factor 6", 10},};

        Option bandwidthOption {
            "Bandwidth",
            "Bandwidth of IF filter",
            bandwidthValues,
            2
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
        Option passbandGainOption {
                "PassbandGain",
                "Gain factor after preselect",
                passbandGainValues,
                20
        };
        Option iqPassbandGainOption {
                "IQPassbandGain",
                "Gain factor after iq-to-real conversion",
                iqPassbandGainValues,
                4
        };

        // Register options
        RegisterOption(bandwidthOption);
        RegisterOption(beattoneOption);
        RegisterOption(ifshiftOption);
        RegisterOption(passbandGainOption);
        RegisterOption(iqPassbandGainOption);
    }

HWriterConsumer<int16_t>* BoomaCwReceiver::PreProcess(ConfigOptions* opts, HWriterConsumer<int16_t>* previous) {
    HLog("Creating CW receiver preprocessing chain");

    // If we get real-valued data, then most likely the source is an audio device
    // or at least, the samplerate will be low and may contain hum
    if(opts->GetInputSourceDataType() == REAL_INPUT_SOURCE_DATA_TYPE ) {

        // Calculate mixer offsets due to if filter shifting
        int offset = GetIfOffset();
        HLog("IF mixer offset due to IF shift set to %dHz", offset);

        // Add a combfilter to kill (more) 50 hz harmonics
        HLog("- Humfilter");
        _humfilterProbe = new HProbe<int16_t>("cwreceiver_01_humfilter", _enableProbes);
        _humfilter = new HHumFilter<int16_t>(previous, opts->GetOutputSampleRate(), 50, 1000, BLOCKSIZE, _humfilterProbe);

        // Bandpass filter before mixing to remove or reduce frequencies we do not want to mix
        HLog("- Preselect");
        _preselectProbe = new HProbe<int16_t>("cwreceiver_02_preselect", _enableProbes);
        _preselect = new HBiQuadFilter<HBandpassBiQuad<int16_t>, int16_t>(_humfilter->Consumer(), GetFrequency() + offset, opts->GetOutputSampleRate(), 1.0f, 1, BLOCKSIZE, _preselectProbe);

        // Gain after preselect filtering
        _passbandGainProbe = new HProbe<int16_t>("cwreceiver_03_preselectgain", _enableProbes);
        _passbandGain = new HGain<int16_t>(_preselect->Consumer(), GetOption("PassbandGain"), BLOCKSIZE, _passbandGainProbe);

        // Mix down to IF frequency = 6000Hz
        HLog("- IF Mixer");
        _ifMixerProbe = new HProbe<int16_t>("cwreceiver_04_if_mixer", _enableProbes);
        _ifMixer = new HMultiplier<int16_t>(_passbandGain->Consumer(), opts->GetOutputSampleRate(), GetFrequency() - 6000 + offset, 10, BLOCKSIZE, _ifMixerProbe);

        // Return signal at IF = 6KHz
        return _ifMixer->Consumer();
    }

    // If we get iq data, then the input spectrum is centered with the tuned frequency at 0
    // so we need to move the (positive) frequency of interest to the IF frequency = 6KHz and
    // convert to realvalued samples at the output samplerate.
    // We do not need to filter away other frequencies, that is handled by the receivers IF filter.
    // Also, since we are decimating IQ samples, there will be nothing outside +- 3KHz, so by
    // moving the center to 6KHz, we translate all negative frequencies to positive.
    if( opts->GetInputSourceDataType() == IQ_INPUT_SOURCE_DATA_TYPE ||
            opts->GetInputSourceDataType() == I_INPUT_SOURCE_DATA_TYPE ||
            opts->GetInputSourceDataType() == Q_INPUT_SOURCE_DATA_TYPE) {

        // Move the center frequency up to 6KHz which is the IF frequency
        _iqMultiplierProbe = new HProbe<int16_t>("cwreceiver_05_iqmultiplier", _enableProbes);
        _iqMultiplier = new HIqMultiplier<int16_t>(previous, opts->GetOutputSampleRate(), 6000, 10, BLOCKSIZE, _iqMultiplierProbe);

        // Get the I branch ==> convert to realvalued samples
        _iq2IConverterProbe = new HProbe<int16_t>("cwreceiver_06_iq2iconverter", _enableProbes);
        _iq2IConverter = new HIq2IConverter<int16_t>(_iqMultiplier->Consumer(), BLOCKSIZE, _iq2IConverterProbe);

        // Gain after converting to realvalued samples
        _passbandGainProbe = new HProbe<int16_t>("cwreceiver_07_preselectgain", _enableProbes);
        _passbandGain = new HGain<int16_t>(_iq2IConverter->Consumer(), GetOption("IQPassbandGain"), BLOCKSIZE, _passbandGainProbe);

        // Return signal at IF = 6KHz
        return _passbandGain->Consumer();
    }

    // Unhandled data type - that should not happen!
    throw new BoomaReceiverException("Unhandled data type");
}

HWriterConsumer<int16_t>* BoomaCwReceiver::Receive(ConfigOptions* opts, HWriterConsumer<int16_t>* previous) {
    HLog("Creating CW receiver receiving chain %d");

    // Calculate mixer offsets due to if filter shifting
    int offset = GetIfOffset();
    HLog("IF mixer offset due to IF shift set to %dHz", offset);

    // Narrow if filter consisting of a number of cascaded 2. order bandpass filters
    HLog("- IF filter");
    _ifFilterProbe = new HProbe<int16_t>("cwreceiver_08_if_filter", _enableProbes);
    _ifFilter = new HCascadedBiQuadFilter<int16_t>(previous, _bandpassCoeffs[GetOption("Bandwidth")], 20, BLOCKSIZE, _ifFilterProbe);

    // Mix down to the output frequency.
    // 6000Hz - 5160Hz = 840Hz
    HLog("- Beat tone mixer");
    _beatToneMixerProbe = new HProbe<int16_t>("cwreceiver_09_beat_tone_mixer", _enableProbes);
    _beatToneMixer = new HMultiplier<int16_t>(_ifFilter->Consumer(), opts->GetOutputSampleRate(), 6000 - GetOption("Beattone") - offset, 10, BLOCKSIZE, _beatToneMixerProbe);

    // Smoother bandpass filter (2 stacked biquads) to remove artifacts from the very narrow detector
    // filter above
    HLog("- Output filter");
    _postSelectProbe = new HProbe<int16_t>("cwreceiver2_10_postselect", _enableProbes);
    _postSelect = new HCascadedBiQuadFilter<int16_t>(_beatToneMixer->Consumer(), _cwCoeffs, 20, BLOCKSIZE, _postSelectProbe);

    // End of receiver
    return _postSelect->Consumer();
}

HWriterConsumer<int16_t>* BoomaCwReceiver::PostProcess(ConfigOptions* opts, HWriterConsumer<int16_t>* previous) {
    HLog("Creating CW receiver postprocessing chain");

    return previous;
}

BoomaCwReceiver::~BoomaCwReceiver() {
    SAFE_DELETE(_humfilter);
    SAFE_DELETE(_preselect);
    SAFE_DELETE(_passbandGain);
    SAFE_DELETE(_ifMixer);
    SAFE_DELETE(_iqMultiplier);
    SAFE_DELETE(_iq2IConverter);
    SAFE_DELETE(_ifFilter);
    SAFE_DELETE(_beatToneMixer);
    SAFE_DELETE(_postSelect);
    SAFE_DELETE(_humfilterProbe);
    SAFE_DELETE(_iqMultiplierProbe);
    SAFE_DELETE(_iq2IConverterProbe);
    SAFE_DELETE(_preselectProbe);
    SAFE_DELETE(_passbandGainProbe);
    SAFE_DELETE(_ifMixerProbe);
    SAFE_DELETE(_ifFilterProbe);
    SAFE_DELETE(_beatToneMixerProbe);
    SAFE_DELETE(_postSelectProbe);
}

bool BoomaCwReceiver::SetInternalFrequency(ConfigOptions* opts, int frequency) {

    // This receiver only operates from 6000 - samplerate/2. Or exactly on o (zero, IQ devices)
    if( !IsFrequencySupported(opts, frequency) ) {
        HError("Unsupported frequency %ld, must be greater than  %d and less than %d or zero", frequency, 6000, opts->GetOutputSampleRate() / 2);
        return false;
    }

    // Calculate mixer offsets due to if filter shifting
    int offset = GetIfOffset();
    HLog("IF mixer offset due to IF shift set to %dHz", offset);

    // Set new multiplier frequency and adjust the preselect bandpass filter
    if( _preselect != nullptr ) {
        _preselect->SetCoefficients(frequency + offset, opts->GetOutputSampleRate(), 1.0f, 1, BLOCKSIZE);
    }
    if( _ifMixer != nullptr ) {
        _ifMixer->SetFrequency(frequency - 6000 + offset);
    }

    // Ready
    return true;
}

void BoomaCwReceiver::OptionChanged(ConfigOptions* opts, std::string name, int value) {
    HLog("Option %s has changed to value %d", name.c_str(), value);

    // Calculate mixer offsets due to if filter shifting
    int offset = GetIfOffset();
    HLog("IF mixer offset due to IF shift set to %dHz", offset);

    // Reconfigure receiver
    if( _preselect != nullptr) {
        _preselect->SetCoefficients(GetFrequency() + offset, opts->GetOutputSampleRate(), 1.0f, 1, BLOCKSIZE);
    }
    if( _ifMixer != nullptr ) {
        _ifMixer->SetFrequency(GetFrequency() - 6000 + offset);
    }

    _ifFilter->SetCoefficients(_bandpassCoeffs[GetOption("Bandwidth")], 20);
    _beatToneMixer->SetFrequency(6000 - GetOption("Beattone") - offset);

    if( _preselect != nullptr ) {
        _passbandGain->SetGain(GetOption("PassbandGain"));
    } else if( _iq2IConverter != nullptr ) {
        if( GetOption("IQPassbandGain") > 0 ) {
            _passbandGain->SetGain(GetOption("IQPassbandGain"));
        } else {
            _passbandGain->SetGain(0.5);
        }
    }

    // Settings applied
    HLog("Receiver chain reconfigured");
}

std::string BoomaCwReceiver::GetOptionInfoString() { 
    std::string info = "bw:" + std::to_string(_bandpassWidths[GetOption("Bandwidth")]) + "Hz";
    if( GetOption("Ifshift") != 0 ) {
        int shift = _bandpassWidths[GetOption("Bandwidth")] / 4;
        info += (GetOption("Ifshift") > 0 ? " " + std::to_string(shift) + "-->" : " <--" + std::to_string(shift));
    } 
    return  info;
}