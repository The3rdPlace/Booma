#include "boomassbreceiver.h"

BoomaSsbReceiver::BoomaSsbReceiver(ConfigOptions* opts, int initialFrequency):
        BoomaReceiver(opts, initialFrequency, true, 3000),
        _enableProbes(opts->GetEnableProbes()),
        _iqMultiplier(nullptr),
        _iqFirFilter(nullptr),
        _iq2IConverter(nullptr),
        _translateByFour(nullptr),
        _lowpassFilter(nullptr),
        _iqMultiplierProbe(nullptr),
        _iqFirFilterProbe(nullptr),
        _iq2IConverterProbe(nullptr),
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
    _iqMultiplier = new HIqMultiplier<int16_t>(previous, GetOutputSamplerate(), 12000 + 1700, 10, BLOCKSIZE, _iqMultiplierProbe);

    // Remove (formerly) negative frequencies by passband filtering
    _iqFirFilterProbe = new HProbe<int16_t>("ssbreceiver_02_iqfirfilter", _enableProbes);
    _iqFirFilter = new HIqFirFilter<int16_t>(_iqMultiplier->Consumer(), HBandpassKaiserBessel<int16_t>(12000, 15000, GetOutputSamplerate(), 15, 50).Calculate(), 15, BLOCKSIZE, _iqFirFilterProbe);

    // Get the I branch ==> convert to realvalued samples
    _iq2IConverterProbe = new HProbe<int16_t>("ssbreceiver_03_iq2iconverter", _enableProbes);
    _iq2IConverter = new HIq2IConverter<int16_t>(_iqFirFilter->Consumer(), BLOCKSIZE, _iq2IConverterProbe);

    // Get back to baseband by multiplying with fs/4
    _translateByFourProbe = new HProbe<int16_t>("ssbreceiver_04_translatebyfour", _enableProbes);
    _translateByFour = new HTranslateByFour<int16_t>(_iq2IConverter->Consumer(), BLOCKSIZE, true, _translateByFourProbe);

    // And finally, filter out the high copy of the spectrum that is created by the translation
    _lowpassFilterProbe = new HProbe<int16_t>("ssbreceiver_05_lowpassfilter", _enableProbes);
    _lowpassFilter = new HBiQuadFilter<HLowpassBiQuad<int16_t>, int16_t>(_translateByFour->Consumer(), 3000, GetOutputSamplerate(), 0.707, 1, BLOCKSIZE, _lowpassFilterProbe);

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
    SAFE_DELETE(_iq2IConverterProbe);
    SAFE_DELETE(_translateByFourProbe);
    SAFE_DELETE(_lowpassFilterProbe);

    SAFE_DELETE(_iqMultiplier);
    SAFE_DELETE(_iqFirFilter);
    SAFE_DELETE(_iq2IConverter);
    SAFE_DELETE(_translateByFour);
    SAFE_DELETE(_lowpassFilter);
}

bool BoomaSsbReceiver::SetFrequency(int frequency) {

    // This receiver operates on all frequencies that can be tuned by the IQ sampler
    return true;
}
