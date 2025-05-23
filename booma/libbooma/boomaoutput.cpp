#include "boomaoutput.h"

BoomaOutput::BoomaOutput(ConfigOptions* opts, BoomaReceiver* receiver):
        _outputVolume(nullptr),
        _outputFilter(nullptr),
        _soundcardMultiplexer(nullptr),
        _soundcardWriter(nullptr),
        _nullWriter(nullptr),
        _audioWriter(nullptr),
        _pcmWriter(nullptr),
        _wavWriter(nullptr),
        _audioSplitter(nullptr),
        _audioBreaker(nullptr),
        _audioBuffer(nullptr),
        _frequencyAlignmentGenerator(nullptr),
        _frequencyAlignmentMixer(nullptr),
        _ifSplitter(nullptr),
        _signalLevel(nullptr),
        _signalLevelWriter(nullptr),
        _outputFilterWidth(receiver->GetOutputFilterWidth()),
        _audioFft(nullptr),
        _audioFftWindow(nullptr),
        _audioFftWriter(nullptr),
        _audioSpectrum(nullptr),
        _audioFftSize(256),
        _audioFftGain(nullptr) {

    // AF fft spectrum output
    _audioSpectrumSize = _audioFftSize / 2;
    _audioSpectrum = new double[_audioSpectrumSize];
    memset((void*) _audioSpectrum, 0, sizeof(double) * _audioSpectrumSize);

    // Final output filter to remove high frequencies
    _outputFilter = new HFirFilter<int16_t>("output_high_frequence_fir", receiver->GetLastWriterConsumer(), HLowpassKaiserBessel<int16_t>(_outputFilterWidth, opts->GetOutputSampleRate(), 15, 90).Calculate(), 15, BLOCKSIZE);

    // Setup a splitter to split off audio dump
    HLog("Setting up output audio splitter");
    _audioSplitter = new HSplitter<int16_t>("output_audio_splitter", _outputFilter->Consumer());
    _audioDelay = new HDelay<int16_t>("output_audio_delay", _audioSplitter->Consumer(), BLOCKSIZE, opts->GetOutputSampleRate(), 10);
    _audioBreaker = new HBreaker<int16_t>("output_audio_breaker", _audioDelay->Consumer(), !opts->GetDumpAudio(), BLOCKSIZE);
    _audioBuffer = new HBufferedWriter<int16_t>("output_audio_buffer", _audioBreaker->Consumer(), BLOCKSIZE, opts->GetReservedBuffers(), opts->GetEnableBuffers());
    std::string dumpfile = "OUTPUT_" + (opts->GetDumpFileSuffix() == "" ? std::to_string(std::time(nullptr)) : opts->GetDumpFileSuffix());
    if( opts->GetDumpAudioFileFormat() == WAV ) {
        _audioWriter = new HWavWriter<int16_t>("output_audio_wav_writer", (dumpfile + ".wav").c_str(), H_SAMPLE_FORMAT_INT_16, 1, opts->GetOutputSampleRate(), _audioBuffer->Consumer(), true);
    } else {
        _audioWriter = new HFileWriter<int16_t>("output_audio_pcm_writer", (dumpfile + ".pcm").c_str(), _audioBuffer->Consumer(), true);
    }

    // Add signallevel measurement just before the volume
    HLog("Setting up signallevel measurement");
    _signalLevel = new HSignalLevelOutput<int16_t>("output_signal_level_splitter", _audioSplitter->Consumer(), SIGNALLEVEL_AVERAGING_COUNT, 54, 16);
    _signalLevelWriter = HCustomWriter<HSignalLevelResult>::Create<BoomaOutput>("output_signal_level_writer", this, &BoomaOutput::SignalLevelCallback, _signalLevel->Consumer());

    // Add audio spectrum calculation
    _audioFftGain = new HAgc<int16_t>("output_spectrum_gain", _audioSplitter->Consumer(), opts->GetAfFftAgcLevel(), 3,  BLOCKSIZE);
    _audioFftWindow = new HHammingWindow<int16_t>();
    _audioFft = new HFftOutput<int16_t>("output_spectrum_fft_output", _audioFftSize, AUDIOFFT_AVERAGING_COUNT, AUDIOFFT_SKIP, _audioFftGain->Consumer(), _audioFftWindow, opts->GetOutputSampleRate(), 4, opts->GetOutputSampleRate() / 16);
    _audioFftWriter = HCustomWriter<HFftResults>::Create<BoomaOutput>("output_spectrum_writer", this, &BoomaOutput::AudioFftCallback, _audioFft->Consumer());

    // Add volume control
    HLog("Output volume");
    _outputVolume = new HGain<int16_t>("output_volume_control", _audioSplitter->Consumer(), opts->GetVolume(), BLOCKSIZE);

    // Enable frequency alignment ?
    if( opts->GetFrequencyAlign() ) {
        HLog("Enabling ftl-sdr frequency alignment mode");
        _frequencyAlignmentGenerator = new HSineGenerator<int16_t>("output_frequency_alignment_generator", opts->GetOutputSampleRate(), 800, opts->GetFrequencyAlignVolume());
        _frequencyAlignmentMixer = new HLinearMixer<int16_t>("output_frequency_alignment_mixer", _frequencyAlignmentGenerator->Reader(), _outputVolume->Consumer(), BLOCKSIZE);
    }

    // Select output device
    if( opts->GetOutputFilename() != "" ) {
        HLog("Writing output audio to %s", opts->GetOutputFilename().c_str());
        if( IsWav(opts->GetOutputFilename()) ) {
            HLog("Creating output wav file");
            _wavWriter = new HWavWriter<int16_t>("output_audio_wav_writer", opts->GetOutputFilename().c_str(), H_SAMPLE_FORMAT_INT_16, 1, opts->GetOutputSampleRate(), GetOutputVolumeConsumer());
            _pcmWriter = nullptr;
        } else {
            HLog("Creating output pcm file");
            _pcmWriter = new HFileWriter<int16_t>("output_audio_pcm_writer", opts->GetOutputFilename().c_str(), GetOutputVolumeConsumer());
            _wavWriter = nullptr;
        }
        _soundcardWriter = nullptr;
        _nullWriter = nullptr;
    }
    else if( opts->GetOutputAudioDevice() == -1 ) {
        HLog("Writing output audio to /dev/null device");
        _nullWriter = new HNullWriter<int16_t>("output_null_writer", GetOutputVolumeConsumer());
        _soundcardWriter = nullptr;
        _pcmWriter = nullptr;
        _wavWriter = nullptr;
    }
    else
    {
        HLog("Initializing multiplexer for 2-channel mono output");
        std::vector<HWriterConsumer<int16_t>*> consumers;
        consumers.push_back(GetOutputVolumeConsumer());
        _soundcardMultiplexer = new HMux<int16_t>("output_multiplexer", consumers, BLOCKSIZE, true);

        HLog("Initializing audio output device %d", opts->GetOutputAudioDevice());
        _soundcardWriter = new HSoundcardWriter<int16_t>("output_audio_card_writer", opts->GetOutputAudioDevice(), opts->GetOutputSampleRate(), 2, H_SAMPLE_FORMAT_INT_16, BLOCKSIZE, _soundcardMultiplexer->Consumer());
        _nullWriter = nullptr;
        _pcmWriter = nullptr;
        _wavWriter = nullptr;
    }
}

BoomaOutput::~BoomaOutput() {

    SAFE_DELETE(_outputVolume);
    SAFE_DELETE(_outputFilter);
    SAFE_DELETE(_soundcardMultiplexer);
    SAFE_DELETE(_soundcardWriter);
    SAFE_DELETE(_nullWriter);
    SAFE_DELETE(_pcmWriter);
    SAFE_DELETE(_wavWriter);
    SAFE_DELETE(_audioWriter);
    SAFE_DELETE(_audioSplitter);
    SAFE_DELETE(_audioBreaker);
    SAFE_DELETE(_audioBuffer);
    SAFE_DELETE(_frequencyAlignmentGenerator);
    SAFE_DELETE(_frequencyAlignmentMixer);
    SAFE_DELETE(_ifSplitter);
    SAFE_DELETE(_signalLevel);
    SAFE_DELETE(_signalLevelWriter);
    SAFE_DELETE(_audioFft);
    SAFE_DELETE(_audioFftWriter);
    SAFE_DELETE(_audioFftWindow);
    SAFE_DELETE(_audioSpectrum);
    SAFE_DELETE(_audioFftGain);
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

bool BoomaOutput::IsWav(std::string filename) {

    // Must have a name and the suffix .wav
    if( filename.length() <= 4 ) {
        return false;
    }

    // Get lowercase version of the filename
    std::string lower = filename;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);

    // Search for (reverse) the suffix .wav
    return lower.rfind(".wav", lower.length() - 4, 4) != std::string::npos;
}


int BoomaOutput::SignalLevelCallback(HSignalLevelResult* result, size_t length) {

    // Store the current level
    _signalStrength = result->S;
    _signalMax = result->Max;

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

int BoomaOutput::GetSignalMax() {
    return _signalMax;
}

int BoomaOutput::AudioFftCallback(HFftResults* result, size_t length) {

    // Store the current spectrum in the output buffer
    memcpy((void*) _audioSpectrum, (void*) result->Spectrum, sizeof(double) * _audioFftSize / 2);
    return length;
}

int BoomaOutput::GetAudioFftSize() {
    return _audioFftSize;
}

int BoomaOutput::GetAudioSpectrum(double* spectrum) {
    memcpy((void*) spectrum, _audioSpectrum, sizeof(double) * _audioSpectrumSize);
    return _audioSpectrumSize;
}
