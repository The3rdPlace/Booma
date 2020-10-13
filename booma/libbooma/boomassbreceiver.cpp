#include "boomassbreceiver.h"

BoomaSsbReceiver::BoomaSsbReceiver(ConfigOptions* opts, int initialFrequency):
        BoomaReceiver(opts, initialFrequency, 3000, true),
        _enableProbes(opts->GetEnableProbes()),
        _iqMultiplier(nullptr),
        _iqFirFilter(nullptr),
        _iqAdder(nullptr),
        _collector(nullptr),
        _translateByFour(nullptr),
        _lowpassFilter(nullptr),
        _iqMultiplierProbe(nullptr),
        _iqFirFilterProbe(nullptr),
        _iqAdderProbe(nullptr),
        _translateByFourProbe(nullptr),
        _lowpassFilterProbe(nullptr) {}

HWriterConsumer<int16_t>* BoomaSsbReceiver::PreProcess(ConfigOptions* opts, HWriterConsumer<int16_t>* previous) {
    HLog("Creating SSB receiver preprocessing chain");
    // -------------------------------------------------------
    HLog("WARNING: This is experimental / work in progress!");
    std::cout << "WARNING: The SSB receiver is in an experimental state and is work-in-progress!" << std::endl;
    // -------------------------------------------------------

    // Move the center frequency up to 12KHz, but subtract 1.7KHz to adjust for USB modulation
    // Todo: switch between LSB and USB
    _iqMultiplierProbe = new HProbe<int16_t>("ssbreceiver_01_iqmultiplier", _enableProbes);
    _iqMultiplier = new HIqMultiplier<int16_t>(previous, opts->GetOutputSampleRate(), 12000 + 850, 10, BLOCKSIZE, _iqMultiplierProbe);

    // Remove (formerly) negative frequencies by passband filtering
    _iqFirFilterProbe = new HProbe<int16_t>("ssbreceiver_02_iqfirfilter", _enableProbes);
    _iqFirFilter = new HIqFirFilter<int16_t>(_iqMultiplier->Consumer(), HBandpassKaiserBessel<int16_t>(12000, 15000, opts->GetOutputSampleRate(), 15, 50).Calculate(), 15, BLOCKSIZE, _iqFirFilterProbe);

    // Demodulate usb or lsb by use of the Weaver or "3rd." method.
    // Todo: fixed on USB for now. Handle switch between LSB and USB
    _iqAdderProbe = new HProbe<int16_t>("ssbreceiver_03_iqadder", _enableProbes);
    _iqAdder = new HIqAddOrSubtractConverter<int16_t>(_iqFirFilter->Consumer(), false, BLOCKSIZE, _iqAdderProbe);

    // The iq-adder returns half the amount of incomming samples which equals BLOCKSIZE/2
    // Get back to the global BLOCKSIZE by collecting two blocks before writing further downstream
    _collector = new HCollector<int16_t>(_iqAdder->Consumer(), BLOCKSIZE / 2, BLOCKSIZE);

    // Get back to baseband by multiplying with fs/4
    _translateByFourProbe = new HProbe<int16_t>("ssbreceiver_04_translatebyfour", _enableProbes);
    _translateByFour = new HTranslateByFour<int16_t>(_collector->Consumer(), BLOCKSIZE, true, _translateByFourProbe);

    // And finally, filter out the high copy of the spectrum that is created by the translation
    _lowpassFilterProbe = new HProbe<int16_t>("ssbreceiver_05_lowpassfilter", _enableProbes);
    _lowpassFilter = new HBiQuadFilter<HLowpassBiQuad<int16_t>, int16_t>(_translateByFour->Consumer(), 3000, opts->GetOutputSampleRate(), 0.707, 1, BLOCKSIZE, _lowpassFilterProbe);

    // Return signal at IF = 6KHz
    return _lowpassFilter->Consumer();
}

HWriterConsumer<int16_t>* BoomaSsbReceiver::Receive(ConfigOptions* opts, HWriterConsumer<int16_t>* previous) {
    HLog("Creating SSB receiving chain");

    return previous;
}

HWriterConsumer<int16_t>* BoomaSsbReceiver::PostProcess(ConfigOptions* opts, HWriterConsumer<int16_t>* previous) {
    HLog("Creating SSB receiver postprocessing chain");

    return previous;
}

BoomaSsbReceiver::~BoomaSsbReceiver() {

    SAFE_DELETE(_iqMultiplierProbe);
    SAFE_DELETE(_iqFirFilterProbe);
    SAFE_DELETE(_iqAdderProbe);
    SAFE_DELETE(_translateByFourProbe);
    SAFE_DELETE(_lowpassFilterProbe);

    SAFE_DELETE(_iqMultiplier);
    SAFE_DELETE(_iqFirFilter);
    SAFE_DELETE(_iqAdder);
    SAFE_DELETE(_collector);
    SAFE_DELETE(_translateByFour);
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
