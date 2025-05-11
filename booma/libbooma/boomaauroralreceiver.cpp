#include "boomaauroralreceiver.h"

BoomaAuroralReceiver::BoomaAuroralReceiver(ConfigOptions* opts, int initialFrequency):
    BoomaReceiver(opts, initialFrequency),
    _humfilter(nullptr),
    _bandpass(nullptr),
    _averaging(nullptr),
    _gaussian(nullptr) {

    opts->SetFrequency(5000);
    opts->SetInputFilterWidth(10000);
    opts->SetRfGain(5);

    std::vector<OptionValue> HumfilterValues {
            OptionValue {"On", "Enable the humfilter", 1},
            OptionValue {"Off", "Disable the humfilter", 0}};
    std::vector<OptionValue> MovingAverageFilterValues {
            OptionValue {"On", "Enable the moving average filter (1-10KHz)", 1},
            OptionValue {"Off", "Disable the moving average filter", 0}};
    std::vector<OptionValue> GaussianFilterValues {
            OptionValue {"On", "Enable the gaussian (noise reduction) filter", 1},
            OptionValue {"Off", "Disable the gaussian (noise reduction) filter", 0}};

    Option HumfilterOption {
            "Humfilter",
            "Humfilter (remove 50Hz and harmonics)",
            HumfilterValues,
            0
    };
    Option MovingAverageFilterOption {
            "MovingAveragefilter",
            "Moving average filter 0 - 10KHz",
            MovingAverageFilterValues,
            0
    };
    Option GaussianFilterOption {
            "Gaussianfilter",
            "Gaussian (noise reduction) filter",
            GaussianFilterValues,
            1
    };

    RegisterOption(HumfilterOption);
    RegisterOption(MovingAverageFilterOption);
    RegisterOption(GaussianFilterOption);
}

HWriterConsumer<int16_t>* BoomaAuroralReceiver::PreProcess(ConfigOptions* opts, HWriterConsumer<int16_t>* previous) {
    HLog("Creating AURORAL receiver preprocessing chain");

    // Add a combfilter to kill (more) 50 hz harmonics
    HLog("Adding 50Hz humfilter for audio device input");
    _humfilter = new HCombFilter<int16_t>("auroral_receiver_pre_process_hum_comb", previous, opts->GetInputSampleRate(), 50, -0.907f, BLOCKSIZE);

    // Narrow bandpass filter, from 100Hz to 10KHz.
    HLog("- Bandpass");
    _bandpass = new HFirFilter<int16_t>("auroral_receiver_pre_process_bandpass_fir", _humfilter->Consumer(), HBandpassKaiserBessel<int16_t>(100, 10000, opts->GetOutputSampleRate(), 115, 96).Calculate(), 115, BLOCKSIZE);

    if( GetOption("Humfilter") == 1 ) {
        _humfilter->Enable();
    } else {
        _humfilter->Disable();
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

    _averaging = new HMovingAverageFilter<int16_t>("auroral_receiver_post_process_averaging", previous, 3, BLOCKSIZE);
    _gaussian = new HGaussianFilter<int16_t>("auroral_receiver_post_process_gaussian", _averaging->Consumer(), BLOCKSIZE, 2, 1024);

    if( GetOption("MovingAveragefilter") == 1 ) {
        _averaging->Enable();
    } else {
        _averaging->Disable();
    }

    if( GetOption("Gaussianfilter") == 1 ) {
        _gaussian->Enable();
    } else {
        _gaussian->Disable();
    }

    return _gaussian->Consumer();
}

BoomaAuroralReceiver::~BoomaAuroralReceiver() {
    SAFE_DELETE(_bandpass);
    SAFE_DELETE(_humfilter);
    SAFE_DELETE(_averaging);
    SAFE_DELETE(_gaussian);
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

    if( name == "MovingAveragefilter" ) {
        if( value == 1 ) {
            HLog("Enabled the moving average filter");
            _averaging->Enable();
        } else {
            HLog("Disabled the moving average filter");
            _averaging->Disable();
        }
    }

    if( name == "Gaussianfilter" ) {
        if( value == 1 ) {
            HLog("Enabled the gaussian (noise reduction) filter");
            _gaussian->Enable();
        } else {
            HLog("Disabled the gaussian (noise reduction) filter");
            _gaussian->Disable();
        }
    }

    // Settings applied
    HLog("Receiver chain reconfigured");
}
