#include "boomassbreceiver.h"

BoomaSsbReceiver::BoomaSsbReceiver(ConfigOptions* opts, int initialFrequency):
        BoomaReceiver(opts, initialFrequency),
        _enableProbes(opts->GetEnableProbes()),
        _inputFirFilter(nullptr),
        _iqMultiplier(nullptr),
        _iqFirFilter(nullptr),
        _basebandMultiplier(nullptr),
        _iqAdder(nullptr),
        _collector(nullptr),
        _lowpassFilter(nullptr),
        _inputFirFilterProbe(nullptr),
        _iqMultiplierProbe(nullptr),
        _iqFirFilterProbe(nullptr),
        _basebandMultiplierProbe(nullptr),
        _iqAdderProbe(nullptr),
        _lowpassFilterProbe(nullptr) {

    std::vector<OptionValue> modeValues {
            OptionValue {"USB", "USB = Upper sidebande", 1},
            OptionValue {"LSB", "LSB = Lower sideband", -1}};

    Option modeOption {
            "Mode",
            "SSB mode",
            modeValues,
            0
    };

    // Register options
    RegisterOption(modeOption);

}

HWriterConsumer<int16_t>* BoomaSsbReceiver::PreProcess(ConfigOptions* opts, HWriterConsumer<int16_t>* previous) {
    HLog("Creating SSB receiver preprocessing chain");

    _inputFirFilterProbe = new HProbe<int16_t>("ssbreceiver_01_inputfirfilter", _enableProbes);
    _inputFirFilter = new HIqFirFilter<int16_t>(previous, HLowpassKaiserBessel<int16_t>(2000, opts->GetOutputSampleRate(), 15, 50).Calculate(), 15, BLOCKSIZE, _inputFirFilterProbe);

    // Move the center frequency up to 3000 (place the carrier at 3KHz)
    _iqMultiplierProbe = new HProbe<int16_t>("ssbreceiver_02_iqmultiplier", _enableProbes);
    _iqMultiplier = new HIqMultiplier<int16_t>(_inputFirFilter->Consumer(), opts->GetOutputSampleRate(), 3000, 10, BLOCKSIZE, _iqMultiplierProbe);

    // Remove (formerly) negative frequencies by passband filtering
    _iqFirFilterProbe = new HProbe<int16_t>("ssbreceiver_03_iqfirfilter", _enableProbes);
    if( GetOption("Mode") > 0) {
        _iqFirFilter = new HIqFirFilter<int16_t>(_iqMultiplier->Consumer(), HBandpassKaiserBessel<int16_t>(3000, 6000, opts->GetOutputSampleRate(), 15, 50).Calculate(), 15, BLOCKSIZE, _iqFirFilterProbe);
    } else {
        _iqFirFilter = new HIqFirFilter<int16_t>(_iqMultiplier->Consumer(), HLowpassKaiserBessel<int16_t>(3000, opts->GetOutputSampleRate(), 15, 50).Calculate(), 15, BLOCKSIZE, _iqFirFilterProbe);
    }

    // Move the carrier back down to zero
    _basebandMultiplierProbe = new HProbe<int16_t>("ssbreceiver_04_basebandmultiplier", _enableProbes);
    _basebandMultiplier = new HIqMultiplier<int16_t>(_iqFirFilter->Consumer(), opts->GetOutputSampleRate(), -3000, 10, BLOCKSIZE, _basebandMultiplierProbe);
    return _basebandMultiplier->Consumer();
}

HWriterConsumer<int16_t>* BoomaSsbReceiver::Receive(ConfigOptions* opts, HWriterConsumer<int16_t>* previous) {
    HLog("Creating SSB receiving chain");

    // Demodulate usb or lsb by use of the Weaver or "3rd." method.
    _iqAdderProbe = new HProbe<int16_t>("ssbreceiver_05_iqadder", _enableProbes);
    _iqAdder = new HIqAddOrSubtractConverter<int16_t>(previous, false, BLOCKSIZE, _iqAdderProbe);

    // The iq-adder returns half the amount of incomming samples which equals BLOCKSIZE/2
    // Get back to the global BLOCKSIZE by collecting two blocks before writing further downstream
    _collector = new HCollector<int16_t>(_iqAdder->Consumer(), BLOCKSIZE / 2, BLOCKSIZE);

    return _collector->Consumer();
}

HWriterConsumer<int16_t>* BoomaSsbReceiver::PostProcess(ConfigOptions* opts, HWriterConsumer<int16_t>* previous) {
    HLog("Creating SSB receiver postprocessing chain");

    // And finally, filter out the high copy of the spectrum that is created by the translation
    _lowpassFilterProbe = new HProbe<int16_t>("ssbreceiver_05_lowpassfilter", _enableProbes);
    _lowpassFilter = new HBiQuadFilter<HLowpassBiQuad<int16_t>, int16_t>(previous, 3000, opts->GetOutputSampleRate(), 0.707, 1, BLOCKSIZE, _lowpassFilterProbe);

    // Return final signal
    return _lowpassFilter->Consumer();
}

BoomaSsbReceiver::~BoomaSsbReceiver() {

    SAFE_DELETE(_inputFirFilterProbe);
    SAFE_DELETE(_iqMultiplierProbe);
    SAFE_DELETE(_iqFirFilterProbe);
    SAFE_DELETE(_basebandMultiplierProbe);
    SAFE_DELETE(_iqAdderProbe);
    SAFE_DELETE(_lowpassFilterProbe);

    SAFE_DELETE(_inputFirFilter);
    SAFE_DELETE(_iqMultiplier);
    SAFE_DELETE(_iqFirFilter);
    SAFE_DELETE(_basebandMultiplier);
    SAFE_DELETE(_iqAdder);
    SAFE_DELETE(_collector);
    SAFE_DELETE(_lowpassFilter);
}

bool BoomaSsbReceiver::SetInternalFrequency(ConfigOptions* opts, int frequency) {

    // This receiver only operates from 0 to samplerate/2.
    if( frequency >= opts->GetOutputSampleRate() / 2  ) {
        HError("Unsupported frequency %ld, must be less than %d", frequency, opts->GetOutputSampleRate() / 2);
        return false;
    }

    return true;
}

void BoomaSsbReceiver::OptionChanged(ConfigOptions* opts, std::string name, int value) {
    HLog("Option %s has changed to value %d", name.c_str(), value);

    if( value > 0 ) {
        _iqFirFilter->SetCoefficients(HBandpassKaiserBessel<int16_t>(3000, 6000, opts->GetOutputSampleRate(), 15, 50).Calculate(), 15);
    } else {
        _iqFirFilter->SetCoefficients(HLowpassKaiserBessel<int16_t>(3000, opts->GetOutputSampleRate(), 15, 50).Calculate(), 15);
    }

    // Settings applied
    HLog("Receiver chain reconfigured");
}

std::string BoomaSsbReceiver::GetOptionInfoString() {
    std::string info = "mode:" + std::string(GetOption("Mode") == 1 ? "USB" : "LSB");
    return  info;
}