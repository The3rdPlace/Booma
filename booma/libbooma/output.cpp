#include "output.h"

BoomaOutput::BoomaOutput(ConfigOptions* opts, BoomaReceiver* receiver):
     _outputWriter(NULL),
     _soundcardWriter(NULL),
     _nullWriter(NULL),
     _audioWriter(NULL),
     _audioSplitter(NULL),
     _audioMute(NULL),
     _signalLevel(NULL),
     _signalLevelWriter(NULL),
     _rfFft(NULL),
     _rfFftWindow(NULL),
     _rfFftWriter(NULL),
     _rfSpectrum(NULL),
     _rfFftSize(256),
     _audioFft(NULL),
     _audioFftWindow(NULL),
     _audioFftWriter(NULL),
     _audioSpectrum(NULL),
     _audioFftSize(256)
{

    // Setup a splitter to split off audio dump and signal level metering
    HLog("Setting up output audio splitter");
    _audioSplitter = new HSplitter<int16_t>(receiver->GetLastWriterConsumer());

    // Add a filewriter so that we can dump audio data on request
    _audioMute = new HMute<int16_t>(_audioSplitter->Consumer(), !opts->GetDumpAudio(), BLOCKSIZE);
    std::string dumpfile = "OUTPUT_" + std::to_string(std::time(nullptr));
    if( opts->GetDumpFileFormat() == WAV ) {
        _audioWriter = new HWavWriter<int16_t>((dumpfile + ".wav").c_str(), H_SAMPLE_FORMAT_INT_16, 1, opts->GetSampleRate(), _audioMute->Consumer());
    } else {
        _audioWriter = new HFileWriter<int16_t>((dumpfile + ".pcm").c_str(), _audioMute->Consumer());
    }

    // Add audio signal level metering (before final volume adjust)
    _signalLevel = new HSignalLevel<int16_t>(_audioSplitter->Consumer(), SIGNALLEVEL_AVERAGING_COUNT, 54, 10);
    _signalLevelWriter = HCustomWriter<HSignalLevelResult>::Create<BoomaOutput>(this, &BoomaOutput::SignalLevelCallback, _signalLevel->Consumer());

    // Add RF spectrum calculation
    _audioFftWindow = new HRectangularWindow<int16_t>();
    _audioFft = new HFft<int16_t>(_audioFftSize, AUDIOFFT_AVERAGING_COUNT, _audioSplitter->Consumer(), _audioFftWindow);
    _audioFftWriter = HCustomWriter<HFftResults>::Create<BoomaOutput>(this, &BoomaOutput::AudioFftCallback, _audioFft->Consumer());
    _audioSpectrum = new double[_audioFftSize / 2];
    memset((void*) _audioSpectrum, 0, sizeof(double) * _audioFftSize / 2);

    // Initialize the audio output and the output gain control (volume)
    HLog("Initializing audio output");
    _outputWriter = new HGain<int16_t>(_audioSplitter->Consumer(), opts->GetVolume(), BLOCKSIZE);
    if( opts->GetOutputAudioDevice() == -1 ) {
        HLog("Writing output audio to /dev/null device");
        _soundcardWriter = NULL;
        _nullWriter = new HNullWriter<int16_t>(_outputWriter->Consumer());
    }
    else
    {
        HLog("Initializing audio output device %d", opts->GetOutputAudioDevice());
        _soundcardWriter = new HSoundcardWriter<int16_t>(opts->GetOutputAudioDevice(), opts->GetSampleRate(), 1, H_SAMPLE_FORMAT_INT_16, BLOCKSIZE, _outputWriter->Consumer());
        _nullWriter = NULL;
    }

    // Add RF spectrum calculation
    _rfFftWindow = new HRectangularWindow<int16_t>();
    _rfFft = new HFft<int16_t>(_rfFftSize, RFFFT_AVERAGING_COUNT, receiver->GetSpectrumConsumer(), _rfFftWindow);
    _rfFftWriter = HCustomWriter<HFftResults>::Create<BoomaOutput>(this, &BoomaOutput::RfFftCallback, _rfFft->Consumer());
    _rfSpectrum = new double[_rfFftSize / 2];
    memset((void*) _rfSpectrum, 0, sizeof(double) * _rfFftSize / 2);
}

BoomaOutput::~BoomaOutput() {

    delete _outputWriter;

    if( _soundcardWriter != NULL ) {
        delete _soundcardWriter;
        _soundcardWriter = NULL;
    }
    if( _nullWriter != NULL ) {
        delete _nullWriter;
        _nullWriter = NULL;
    }

    delete _audioWriter;
    delete _audioSplitter;
    delete _audioMute;
    delete _signalLevel;
    delete _signalLevelWriter;
    delete _rfFft;
    delete _rfFftWriter;
    delete _rfFftWindow;
    delete _rfSpectrum;
    delete _audioFft;
    delete _audioFftWriter;
    delete _audioFftWindow;
    delete _audioSpectrum;
}

int BoomaOutput::SignalLevelCallback(HSignalLevelResult* result, size_t length) {

    // Store the current level
    _signalStrength = result->S;
    return length;
}

int BoomaOutput::GetSignalLevel() {
    return _signalStrength;
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
    _audioMute->SetMuted(enabled);
    return _audioMute->GetMuted();
}

int BoomaOutput::SetVolume(int volume) {
    if( volume >= 10 || volume <= 0 ) {
        return false;
    }
    _outputWriter->SetGain(volume);
    return _outputWriter->GetGain();
}