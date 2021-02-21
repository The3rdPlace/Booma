#include "boomaoutput.h"

BoomaOutput::BoomaOutput(ConfigOptions* opts, BoomaReceiver* receiver):
        _outputVolume(nullptr),
        _outputFilter(nullptr),
        _soundcardMultiplexer(nullptr),
        _soundcardWriter(nullptr),
        _nullWriter(nullptr),
        _audioWriter(nullptr),
        _audioSplitter(nullptr),
        _audioBreaker(nullptr),
        _audioBuffer(nullptr),
        _signalLevel(nullptr),
        _signalLevelWriter(nullptr),
        _outputVolumeProbe(nullptr),
        _outputFilterProbe(nullptr) {

    // Setup a splitter to split off audio dump and signal level metering
    HLog("Setting up output audio splitter");
    _audioSplitter = new HSplitter<int16_t>(receiver->GetLastWriterConsumer());

    // Add a filewriter so that we can dump audio data on request
    _audioBreaker = new HBreaker<int16_t>(_audioSplitter->Consumer(), !opts->GetDumpAudio(), BLOCKSIZE);
    _audioBuffer = new HBufferedWriter<int16_t>(_audioBreaker->Consumer(), BLOCKSIZE, opts->GetReservedBuffers(), opts->GetEnableBuffers());
    std::string dumpfile = "OUTPUT_" + (opts->GetDumpFileSuffix() == "" ? std::to_string(std::time(nullptr)) : opts->GetDumpFileSuffix());
    if( opts->GetDumpAudioFileFormat() == WAV ) {
        _audioWriter = new HWavWriter<int16_t>((dumpfile + ".wav").c_str(), H_SAMPLE_FORMAT_INT_16, 1, opts->GetOutputSampleRate(), _audioBuffer->Consumer());
    } else {
        _audioWriter = new HFileWriter<int16_t>((dumpfile + ".pcm").c_str(), _audioBuffer->Consumer());
    }

    // Add audio signal level metering (before final volume adjust)
    _signalLevel = new HSignalLevelOutput<int16_t>(_audioSplitter->Consumer(), SIGNALLEVEL_AVERAGING_COUNT, 54, 10);
    _signalLevelWriter = HCustomWriter<HSignalLevelResult>::Create<BoomaOutput>(this, &BoomaOutput::SignalLevelCallback, _signalLevel->Consumer());

    // Add volume control
    HLog("- Output volume");
    _outputVolumeProbe = new HProbe<int16_t>("output_01_output_volume", opts->GetEnableProbes());
    _outputVolume = new HGain<int16_t>(_audioSplitter->Consumer(), opts->GetVolume(), BLOCKSIZE, _outputVolumeProbe);

    // Final output filter to remove high frequencies
    _outputFilterProbe = new HProbe<int16_t>("output_02_output_filter", opts->GetEnableProbes());
    _outputFilter = new HFirFilter<int16_t>(_outputVolume->Consumer(), HLowpassKaiserBessel<int16_t>(4000, opts->GetOutputSampleRate(), 15, 90).Calculate(), 15, BLOCKSIZE, _outputFilterProbe);

    // Select output device
    if( opts->GetOutputFilename() != "" ) {
        HLog("Writing output audio to %s", opts->GetOutputFilename().c_str());
        if( IsWav(opts->GetOutputFilename()) ) {
            HLog("Creating output wav file");
            _wavWriter = new HWavWriter<int16_t>(opts->GetOutputFilename().c_str(), H_SAMPLE_FORMAT_INT_16, 1, opts->GetOutputSampleRate(), _outputFilter->Consumer());
            _pcmWriter = nullptr;
        } else {
            HLog("Creating output pcm file");
            _pcmWriter = new HFileWriter<int16_t>(opts->GetOutputFilename().c_str(), _outputFilter->Consumer());
            _wavWriter = nullptr;
        }
        _soundcardWriter = nullptr;
        _nullWriter = nullptr;
    }
    else if( opts->GetOutputAudioDevice() == -1 ) {
        HLog("Writing output audio to /dev/null device");
        _nullWriter = new HNullWriter<int16_t>(_outputFilter->Consumer());
        _soundcardWriter = nullptr;
        _pcmWriter = nullptr;
        _wavWriter = nullptr;
    }
    else
    {
        HLog("Initializing multiplexer for 2-channel mono output");
        std::vector<HWriterConsumer<int16_t>*> consumers;
        consumers.push_back(_outputFilter->Consumer());
        _soundcardMultiplexer = new HMux<int16_t>(consumers, BLOCKSIZE, true);

        HLog("Initializing audio output device %d", opts->GetOutputAudioDevice());
        _soundcardWriter = new HSoundcardWriter<int16_t>(opts->GetOutputAudioDevice(), opts->GetOutputSampleRate(), 2, H_SAMPLE_FORMAT_INT_16, BLOCKSIZE, _soundcardMultiplexer->Consumer());
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
    SAFE_DELETE(_signalLevel);
    SAFE_DELETE(_signalLevelWriter);

    SAFE_DELETE(_outputVolumeProbe);
    SAFE_DELETE(_outputFilterProbe);
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
