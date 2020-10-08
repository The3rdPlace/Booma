#include "boomaoutput.h"

BoomaOutput::BoomaOutput(ConfigOptions* opts, BoomaReceiver* receiver):
        _outputAgc(nullptr),
        _outputVolume(nullptr),
        _outputFilter(nullptr),
        _soundcardWriter(nullptr),
        _nullWriter(nullptr),
        _audioWriter(nullptr),
        _audioSplitter(nullptr),
        _audioBreaker(nullptr),
        _audioBuffer(nullptr),
        _signalLevel(nullptr),
        _signalLevelWriter(nullptr),
        _outputAgcProbe(nullptr),
        _outputVolumeProbe(nullptr),
        _outputFilterProbe(nullptr) {

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

    // Add AGC and volume control
    HLog("- AGC + Volume");
    _outputAgcProbe = new HProbe<int16_t>("output_01_output_agc", opts->GetEnableProbes());
    _outputAgc = new HAgc<int16_t>(_audioSplitter->Consumer(), 100, 200, 20, 3, BLOCKSIZE, _outputAgcProbe);
    _outputVolumeProbe = new HProbe<int16_t>("output_02_output_volume", opts->GetEnableProbes());
    _outputVolume = new HGain<int16_t>(_outputAgc->Consumer(), opts->GetVolume(), BLOCKSIZE, _outputVolumeProbe);

    // Final output filter to remove high frequencies
    _outputFilterProbe = new HProbe<int16_t>("output_03_output_filter", opts->GetEnableProbes());
    _outputFilter = new HFirFilter<int16_t>(_outputVolume->Consumer(), HLowpassKaiserBessel<int16_t>(4000, opts->GetOutputSampleRate(), 15, 90).Calculate(), 15, BLOCKSIZE, _outputFilterProbe);

    // Select output device
    if( opts->GetOutputAudioDevice() == -1 ) {
        HLog("Writing output audio to /dev/null device");
        _soundcardWriter = NULL;
        _nullWriter = new HNullWriter<int16_t>(_outputFilter->Consumer());
    }
    else
    {
        HLog("Initializing audio output device %d", opts->GetOutputAudioDevice());
        _soundcardWriter = new HSoundcardWriter<int16_t>(opts->GetOutputAudioDevice(), opts->GetOutputSampleRate(), 1, H_SAMPLE_FORMAT_INT_16, BLOCKSIZE, _outputFilter->Consumer());
        _nullWriter = NULL;
    }
}

BoomaOutput::~BoomaOutput() {

    SAFE_DELETE(_outputAgc);
    SAFE_DELETE(_outputVolume);
    SAFE_DELETE(_outputFilter);

    SAFE_DELETE(_soundcardWriter);
    SAFE_DELETE(_nullWriter);

    SAFE_DELETE(_audioWriter);
    SAFE_DELETE(_audioSplitter);
    SAFE_DELETE(_audioBreaker);
    SAFE_DELETE(_audioBuffer);
    SAFE_DELETE(_signalLevel);
    SAFE_DELETE(_signalLevelWriter);

    SAFE_DELETE(_outputAgcProbe);
    SAFE_DELETE(_outputVolumeProbe);
    SAFE_DELETE(_outputFilterProbe);
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

