#include "boomaauroralreceiver.h"

BoomaAuroralReceiver::BoomaAuroralReceiver(ConfigOptions* opts, int initialFrequency):
    BoomaReceiver(opts, initialFrequency),
    _enableProbes(opts->GetEnableProbes()),
    _humfilterProbe(nullptr),
    _humfilter(nullptr) {

    opts->SetFrequency(5000);
    opts->SetInputFilterWidth(10000);
    opts->SetRfGain(5);

    std::vector<OptionValue> HumfilterValues {
            OptionValue {"On", "Enable the humfilter", 1},
            OptionValue {"Off", "Disable the humfilter", 0}};
    std::vector<OptionValue> HighpassFilterValues {
            OptionValue {"On", "Enable the highpass filter (1-10KHz)", 1},
            OptionValue {"Off", "Disable the highpass filter", 0}};

    Option HumfilterOption {
            "Humfilter",
            "Humfilter (remove 50Hz and harmonics)",
            HumfilterValues,
            1
    };
    Option HighpassFilterOption {
            "Bandpassfilter",
            "Bandpass filter 0 - 10KHz",
            HighpassFilterValues,
            1
    };

    RegisterOption(HumfilterOption);
    RegisterOption(HighpassFilterOption);
}

HWriterConsumer<int16_t>* BoomaAuroralReceiver::PreProcess(ConfigOptions* opts, HWriterConsumer<int16_t>* previous) {
    HLog("Creating AURORAL receiver preprocessing chain");

    // Add a combfilter to kill (more) 50 hz harmonics
    HLog("Adding 50Hz humfilter for audio device input");
    _humfilterProbe = new HProbe<int16_t>("auroralreceiver_01_humfilter", _enableProbes);
    _humfilter = new HCombFilter<int16_t>(previous, opts->GetInputSampleRate(), 50, -0.907f, BLOCKSIZE, _humfilterProbe);

    // Narrow butterworth bandpass filter, from 1KHz to 10KHz.
    HLog("- Bandpass");
    _bandpassProbe = new HProbe<int16_t>("auroralreceiver_02_bandpass", _enableProbes);
    _bandpass = new HFirFilter<int16_t>(_humfilter->Consumer(), HBandpassKaiserBessel<int16_t>(100, 10000, opts->GetOutputSampleRate(), 115, 96).Calculate(), 115, BLOCKSIZE, _bandpassProbe);

    if( GetOption("Humfilter") == 1 ) {
        _humfilter->Enable();
    } else {
        _humfilter->Disable();
    }

    if( GetOption("Bandpassfilter") == 1 ) {
        _bandpass->Enable();
    } else {
        _bandpass->Disable();
    }

    return _bandpass->Consumer();
}

HWriterConsumer<int16_t>* BoomaAuroralReceiver::Receive(ConfigOptions* opts, HWriterConsumer<int16_t>* previous) {
    HLog("Creating AURORAL receiving chain");

    // End of receiver
    return previous;
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

bool BoomaAuroralReceiver::SetInternalFrequency(ConfigOptions* opts, int frequency) {
    return false;
}

void BoomaAuroralReceiver::OptionChanged(ConfigOptions* opts, std::string name, int value) {
    HLog("Option %s has changed to value %d", name.c_str(), value);

    if( name == "Humfilter" ) {
        if( value == 1 ) {
            HLog("Enabled the humfilter");
            _humfilter->Enable();
        } else {
            HLog("Disabled the humfilter");
            _humfilter->Disable();
        }
    }

    if( name == "Bandpassfilter" ) {
        if( value == 1 ) {
            HLog("Enabled the bandpass filter");
            _bandpass->Enable();
        } else {
            HLog("Disabled the andpass filter");
            _bandpass->Disable();
        }
    }

    // Settings applied
    HLog("Receiver chain reconfigured");
}
