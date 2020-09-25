#include "boomaamreceiver.h"

BoomaAmReceiver::BoomaAmReceiver(ConfigOptions* opts, int initialFrequency):
        BoomaReceiver(opts, initialFrequency),
        _enableProbes(opts->GetEnableProbes()),
        _humfilterProbe(nullptr) {}

HWriterConsumer<int16_t>* BoomaAmReceiver::PreProcess(ConfigOptions* opts, HWriterConsumer<int16_t>* previous) {
    HLog("Creating AM receiver preprocessing chain");

    _fdec = new HIqFirDecimator<int16_t>(previous, 8, HLowpassKaiserBessel<int16_t>(5000, 1152000, 51, 50).Calculate(), 51, BLOCKSIZE);
    _dec = new HIqDecimator<int16_t>(_fdec->Consumer(), 3, BLOCKSIZE);
    _conv = new HIq2AbsConverter<int16_t>(_dec->Consumer(), BLOCKSIZE);
    _coll = new HCollector<int16_t>(_conv->Consumer(), BLOCKSIZE / 2, BLOCKSIZE);
    return _coll->Consumer();

}

HWriterConsumer<int16_t>* BoomaAmReceiver::Receive(ConfigOptions* opts, HWriterConsumer<int16_t>* previous) {
    HLog("Creating AM receiving chain");

    // End of receiver
    return previous;
}

HWriterConsumer<int16_t>* BoomaAmReceiver::PostProcess(ConfigOptions* opts, HWriterConsumer<int16_t>* previous) {
    HLog("Creating AM receiver postprocessing chain");

    return previous;
}

BoomaAmReceiver::~BoomaAmReceiver() {

    delete _fdec;
    delete _dec;
    delete _conv;
    delete _coll;
}

bool BoomaAmReceiver::SetFrequency(int frequency) {

    // This receiver only operates from 0 - 10KHz
    if( frequency >= 10000 || frequency <= 0 ) {
        HError("Unsupported frequency %ld, must be greater than  %d and less than %d", frequency, 0, 10000);
        return false;
    }

    // Ready
    return true;
}
