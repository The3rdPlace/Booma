#include "boomaoutput.h"

BoomaOutput::BoomaOutput(ConfigOptions* opts, BoomaReceiver* receiver):
        _outputAgc(nullptr),
        _outputVolume(nullptr),
        _loFilter1(nullptr),
        _loFilter2(nullptr),
        _soundcardWriter(nullptr),
        _nullWriter(nullptr),
        _audioWriter(nullptr),
        _audioSplitter(nullptr),
        _audioBreaker(nullptr),
        _audioBuffer(nullptr),
        _signalLevel(nullptr),
        _signalLevelWriter(nullptr),
        _rfFft(nullptr),
        _rfFftWindow(nullptr),
        _rfFftWriter(nullptr),
        _rfSpectrum(nullptr),
        _rfFftSize(256),
        _audioFft(nullptr),
        _audioFftWindow(nullptr),
        _audioFftWriter(nullptr),
        _audioSpectrum(nullptr),
        _audioFftSize(256),
        _outputAgcProbe(nullptr),
        _outputVolumeProbe(nullptr),
        _loFilterProbe1(nullptr),
        _loFilterProbe2(nullptr),
        _loFilterProbe(nullptr) {

    // Setup a splitter to split off audio dump and signal level metering
    HLog("Setting up output audio splitter");
    _audioSplitter = new HSplitter<int16_t>(receiver->GetLastWriterConsumer());

    // Add a filewriter so that we can dump audio data on request
    _audioBreaker = new HBreaker<int16_t>(_audioSplitter->Consumer(), !opts->GetDumpAudio(), BLOCKSIZE);
    _audioBuffer = new HBufferedWriter<int16_t>(_audioBreaker->Consumer(), BLOCKSIZE, opts->GetReservedBuffers(), opts->GetEnableBuffers());
    std::string dumpfile = "OUTPUT_" + std::to_string(std::time(nullptr));
    if( opts->GetDumpAudioFileFormat() == WAV ) {
        _audioWriter = new HWavWriter<int16_t>((dumpfile + ".wav").c_str(), H_SAMPLE_FORMAT_INT_16, 1, opts->GetOutputSampleRate(), _audioBuffer->Consumer());
    } else {
        _audioWriter = new HFileWriter<int16_t>((dumpfile + ".pcm").c_str(), _audioBuffer->Consumer());
    }

    // Add audio signal level metering (before final volume adjust)
    _signalLevel = new HSignalLevelOutput<int16_t>(_audioSplitter->Consumer(), SIGNALLEVEL_AVERAGING_COUNT, 54, 10);
    _signalLevelWriter = HCustomWriter<HSignalLevelResult>::Create<BoomaOutput>(this, &BoomaOutput::SignalLevelCallback, _signalLevel->Consumer());

    // Add audio spectrum calculation
    _audioFftWindow = new HRectangularWindow<int16_t>();
    _audioFft = new HFftOutput<int16_t>(_audioFftSize, AUDIOFFT_AVERAGING_COUNT, AUDIOFFT_SKIP, _audioSplitter->Consumer(), _audioFftWindow);
    _audioFftWriter = HCustomWriter<HFftResults>::Create<BoomaOutput>(this, &BoomaOutput::AudioFftCallback, _audioFft->Consumer());
    _audioSpectrum = new double[_audioFftSize / 2];
    memset((void*) _audioSpectrum, 0, sizeof(double) * _audioFftSize / 2);

    // Add AGC and volume control
    HLog("- AGC + Volume");
    _outputAgcProbe = new HProbe<int16_t>("output_01_output_agc", opts->GetEnableProbes());
    _outputAgc = new HAgc<int16_t>(_audioSplitter->Consumer(), 150, 200, 10, 5, BLOCKSIZE, _outputAgcProbe);
    _outputVolumeProbe = new HProbe<int16_t>("output_02_output_volume", opts->GetEnableProbes());
    _outputVolume = new HGain<int16_t>(_outputAgc->Consumer(), opts->GetVolume(), BLOCKSIZE, _outputVolumeProbe);
    _loFilterProbe1 = new HProbe<int16_t>("output_03_lo_filter_1", opts->GetEnableProbes());
    _loFilter1 = new HBiQuadFilter<HLowpassBiQuad<int16_t>, int16_t>(_outputVolume->Consumer(), 3000, opts->GetOutputSampleRate(), 0.707, 1, BLOCKSIZE, _loFilterProbe1);
    _loFilterProbe2 = new HProbe<int16_t>("output_04_lo_filter_2", opts->GetEnableProbes());
    _loFilter2 = new HBiQuadFilter<HNotchBiQuad<int16_t>, int16_t>(_loFilter1->Consumer(), opts->GetRtlsdrOffset(), opts->GetOutputSampleRate(), 0.707, 1, BLOCKSIZE, _loFilterProbe2);

    // Select output device
    if( opts->GetOutputAudioDevice() == -1 ) {
        HLog("Writing output audio to /dev/null device");
        _soundcardWriter = NULL;
        _nullWriter = new HNullWriter<int16_t>(_loFilter2->Consumer());
    }
    else
    {
        HLog("Initializing audio output device %d", opts->GetOutputAudioDevice());
        _soundcardWriter = new HSoundcardWriter<int16_t>(opts->GetOutputAudioDevice(), opts->GetOutputSampleRate(), 1, H_SAMPLE_FORMAT_INT_16, BLOCKSIZE, _loFilter2->Consumer());
        _nullWriter = NULL;
    }

    // Add RF spectrum calculation
    _rfFftWindow = new HRectangularWindow<int16_t>();
    _rfFft = new HFftOutput<int16_t>(_rfFftSize, RFFFT_AVERAGING_COUNT, RFFFT_SKIP, receiver->GetSpectrumConsumer(), _rfFftWindow);
    _rfFftWriter = HCustomWriter<HFftResults>::Create<BoomaOutput>(this, &BoomaOutput::RfFftCallback, _rfFft->Consumer());
    _rfSpectrum = new double[_rfFftSize / 2];
    memset((void*) _rfSpectrum, 0, sizeof(double) * _rfFftSize / 2);
}

BoomaOutput::~BoomaOutput() {

    SAFE_DELETE(_outputAgc);
    SAFE_DELETE(_outputVolume);
    SAFE_DELETE(_loFilter1);
    SAFE_DELETE(_loFilter2);

    SAFE_DELETE(_soundcardWriter);
    SAFE_DELETE(_nullWriter);

    SAFE_DELETE(_audioWriter);
    SAFE_DELETE(_audioSplitter);
    SAFE_DELETE(_audioBreaker);
    SAFE_DELETE(_audioBuffer);
    SAFE_DELETE(_signalLevel);
    SAFE_DELETE(_signalLevelWriter);
    SAFE_DELETE(_rfFft);
    SAFE_DELETE(_rfFftWriter);
    SAFE_DELETE(_rfFftWindow);
    SAFE_DELETE(_rfSpectrum);
    SAFE_DELETE(_audioFft);
    SAFE_DELETE(_audioFftWriter);
    SAFE_DELETE(_audioFftWindow);
    SAFE_DELETE(_audioSpectrum);

    SAFE_DELETE(_outputAgcProbe);
    SAFE_DELETE(_outputVolumeProbe);
    SAFE_DELETE(_loFilterProbe1);
    SAFE_DELETE(_loFilterProbe2);
    SAFE_DELETE(_loFilterProbe);
}

int BoomaOutput::SignalLevelCallback(HSignalLevelResult* result, size_t length) {

    // Store the current level
    _signalStrength = result->S;

    // Store the signal sum, scaled
    _signalSum = (int) (result->Sum / BLOCKSIZE);
    return length;
}

int BoomaOutput::GetSignalLevel() {
    return _signalStrength;
}

int BoomaOutput::GetSignalSum() {
    return _signalSum;
}

int BoomaOutput::RfFftCallback(HFftResults* result, size_t length) {

    // Store the current spectrum in the output buffer
    memcpy((void*) _rfSpectrum, (void*) result->Spectrum, sizeof(double) * _rfFftSize / 2);
    return length;
}

int BoomaOutput::GetRfFftSize() {
    return _rfFftSize / 2;
}

int BoomaOutput::GetRfSpectrum(double* spectrum) {
    memcpy((void*) spectrum, _rfSpectrum, sizeof(double) * _rfFftSize / 2);
    return _rfFftSize / 2;
}

int BoomaOutput::AudioFftCallback(HFftResults* result, size_t length) {

    // Store the current spectrum in the output buffer
    memcpy((void*) _audioSpectrum, (void*) result->Spectrum, sizeof(double) * _audioFftSize / 2);
    return length;
}

int BoomaOutput::GetAudioFftSize() {
    return _audioFftSize / 2;
}

int BoomaOutput::GetAudioSpectrum(double* spectrum) {
    memcpy((void*) spectrum, _audioSpectrum, sizeof(double) * _audioFftSize / 2);
    return _audioFftSize / 2;
}

bool BoomaOutput::SetDumpAudio(bool enabled) {
    _audioBreaker->SetOff(!enabled);
    return !_audioBreaker->GetOff();
}

int BoomaOutput::SetVolume(int volume) {
    if( volume > 100 || volume < 0 ) {
        return false;
    }
    _outputVolume->SetGain(volume);
    return _outputVolume->GetGain();
}

