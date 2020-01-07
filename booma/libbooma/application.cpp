#include "application.h"
#include "cwreceiver.h"
#include "booma.h"

BoomaApplication::BoomaApplication(std::string appName, std::string appVersion, int argc, char** argv):
    _opts(NULL),
    _current(NULL),
    _outputWriter(NULL),
    _soundcardWriter(NULL),
    _nullWriter(NULL),
    _audioWriter(NULL),
    _audioSplitter(NULL),
    _audioMute(NULL),
    _receiver(NULL),
    _isRunning(false),
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

    // Initialize the Hardt toolkit.
    // Set the last argument to 'true' to enable verbose output instead of logging to a local file
    HInit(std::string("Booma"), ConfigOptions::IsVerbose(argc, argv));

    // Show library name and and Hardt version.
    HLog("booma: using Hardt %s", getversion().c_str());

    // Parse input arguments
    _opts = new ConfigOptions(appName, appVersion, argc, argv);

    // Initialize receiver
    InitializeReceiver();
}

BoomaApplication::~BoomaApplication() {
    if( _opts != NULL ) {
        delete _opts;
    }
}

bool BoomaApplication::ChangeReceiver() {
    return ChangeReceiver(_opts->GetReceiverModeType());
}

bool BoomaApplication::ChangeReceiver(ReceiverModeType receiverModeType) {

    // Make sure that a running receiver has been shut down
    HLog("Shutting down a running receiver (should we have one)");
    Halt();

    // Register new receiver type
    HLog("Setting new receiver type");
    _opts->SetReceiverModeType(receiverModeType);

    // Reset all previous receiver components
    if( _outputWriter  != NULL ) {
        delete _outputWriter;
        _outputWriter = NULL;
    }
    if( _soundcardWriter != NULL ) {
        delete _soundcardWriter;
        _soundcardWriter = NULL;
    }
    if( _nullWriter != NULL ) {
        delete _nullWriter;
        _nullWriter = NULL;
    }
    if( _audioWriter != NULL ) {
        delete _audioWriter;
        _audioWriter = NULL;
    }
    if( _audioSplitter != NULL ) {
        delete _audioSplitter;
        _audioSplitter = NULL;
    }
    if( _audioMute != NULL ) {
        delete _audioMute;
        _audioMute = NULL;
    }
    if( _signalLevel != NULL ) {
        delete _signalLevel;
        _signalLevel = NULL;
    }
    if( _signalLevelWriter != NULL ) {
        delete _signalLevelWriter;
        _signalLevelWriter = NULL;
    }
    if( _rfFft != NULL ) {
        delete _rfFft;
        _rfFft = NULL;
    }
    if( _rfFftWriter != NULL ) {
        delete _rfFftWriter;
        _rfFftWriter = NULL;
    }
    if( _rfFftWindow != NULL ) {
        delete _rfFftWindow;
        _rfFftWindow = NULL;
    }
    if( _rfSpectrum != NULL ) {
        delete _rfSpectrum;
        _rfSpectrum = NULL;
    }
    if( _audioFft != NULL ) {
        delete _audioFft;
        _audioFft = NULL;
    }
    if( _audioFftWriter != NULL ) {
        delete _audioFftWriter;
        _audioFftWriter = NULL;
    }
    if( _audioFftWindow != NULL ) {
        delete _audioFftWindow;
        _audioFftWindow = NULL;
    }
    if( _audioSpectrum != NULL ) {
        delete _audioSpectrum;
        _audioSpectrum = NULL;
    }

    delete _input;
    delete _receiver;
    delete _output;

    // Configure new receiver
    HLog("Creating new receiver");
    if( !InitializeReceiver() ) {
        HError("Failed to create new receiver");
        return false;
    }
    return true;
}

bool BoomaApplication::InitializeReceiver() {

    // Setup input
    _input = new BoomaInput(_opts, &_isTerminated);

    // If we have a remote head, then we have neither a receiver of output
    if( _opts->GetUseRemoteHead() ) {
        HLog("Using remote head, no local receiver and output");
        return true;
    }

    // Create receiver
    switch( _opts->GetReceiverModeType() ) {
        case CW:
            _receiver = new BoomaCwReceiver(_opts, _input);
            return true;
        default:
            std::cout << "Unknown receiver type defined" << std::endl;
            return false;
    }

    // Setup output
    _output = new BoomaOutput(_opts);
    if( !SetOutput() ) {
        HError("Failed to configure output");
        exit(1);
    }

    // Complete input-receiver-output chain configured
    return true;
}

bool BoomaApplication::SetOutput() {

    // If we have a remote head, then do nothing
    if( _opts->GetUseRemoteHead() ) {
        HLog("Using remote head, no local output");
        return true;
    }

    // Setup a splitter to split off audio dump and signal level metering
    HLog("Setting up output audio splitter");
    _audioSplitter = new HSplitter<int16_t>(_receiver->GetLastWriterConsumer());

    // Add a filewriter so that we can dump audio data on request
    _audioMute = new HMute<int16_t>(_audioSplitter->Consumer(), !_opts->GetDumpAudio(), BLOCKSIZE);
    std::string dumpfile = "OUTPUT_" + std::to_string(std::time(nullptr));
    if( _opts->GetDumpFileFormat() == WAV ) {
        _audioWriter = new HWavWriter<int16_t>((dumpfile + ".wav").c_str(), H_SAMPLE_FORMAT_INT_16, 1, _opts->GetSampleRate(), _audioMute->Consumer());
    } else {
        _audioWriter = new HFileWriter<int16_t>((dumpfile + ".pcm").c_str(), _audioMute->Consumer());
    }

    // Add audio signal level metering (before final volume adjust)
    _signalLevel = new HSignalLevel<int16_t>(_audioSplitter->Consumer(), SIGNALLEVEL_AVERAGING_COUNT, 54, 10);
    _signalLevelWriter = HCustomWriter<HSignalLevelResult>::Create<BoomaApplication>(this, &BoomaApplication::SignalLevelCallback, _signalLevel->Consumer());

    // Add RF spectrum calculation
    _audioFftWindow = new HRectangularWindow<int16_t>();
    _audioFft = new HFft<int16_t>(_audioFftSize, AUDIOFFT_AVERAGING_COUNT, _audioSplitter->Consumer(), _audioFftWindow);
    _audioFftWriter = HCustomWriter<HFftResults>::Create<BoomaApplication>(this, &BoomaApplication::AudioFftCallback, _audioFft->Consumer());
    _audioSpectrum = new double[_audioFftSize / 2];
    memset((void*) _audioSpectrum, 0, sizeof(double) * _audioFftSize / 2);

    // Initialize the audio output and the output gain control (volume)
    HLog("Initializing audio output");
    _outputWriter = new HGain<int16_t>(_audioSplitter->Consumer(), _opts->GetVolume(), BLOCKSIZE);
    if( _opts->GetOutputAudioDevice() == -1 ) {
        HLog("Writing output audio to /dev/null device");
        _soundcardWriter = NULL;
        _nullWriter = new HNullWriter<int16_t>(_outputWriter->Consumer());
    }
    else
    {
        HLog("Initializing audio output device %d", _opts->GetOutputAudioDevice());
        _soundcardWriter = new HSoundcardWriter<int16_t>(_opts->GetOutputAudioDevice(), _opts->GetSampleRate(), 1, H_SAMPLE_FORMAT_INT_16, BLOCKSIZE, _outputWriter->Consumer());
        _nullWriter = NULL;
    }

    // Add RF spectrum calculation
    _rfFftWindow = new HRectangularWindow<int16_t>();
    _rfFft = new HFft<int16_t>(_rfFftSize, RFFFT_AVERAGING_COUNT, _receiver->GetSpectrumConsumer(), _rfFftWindow);
    _rfFftWriter = HCustomWriter<HFftResults>::Create<BoomaApplication>(this, &BoomaApplication::RfFftCallback, _rfFft->Consumer());
    _rfSpectrum = new double[_rfFftSize / 2];
    memset((void*) _rfSpectrum, 0, sizeof(double) * _rfFftSize / 2);

    // Output configured
    return true;
}

int BoomaApplication::SignalLevelCallback(HSignalLevelResult* result, size_t length) {

    // Store the current level
    _signalStrength = result->S;
    return length;
}

int BoomaApplication::GetSignalLevel() {
    return _signalStrength;
}

int BoomaApplication::RfFftCallback(HFftResults* result, size_t length) {

    // Store the current spectrum in the output buffer
    memcpy((void*) _rfSpectrum, (void*) result->Spectrum, sizeof(double) * _rfFftSize / 2);
    return length;
}

int BoomaApplication::GetRfFftSize() {
    return _rfFftSize / 2;
}

int BoomaApplication::GetRfSpectrum(double* spectrum) {
    memcpy((void*) spectrum, _rfSpectrum, sizeof(double) * _rfFftSize / 2);
    return _rfFftSize / 2;
}

int BoomaApplication::AudioFftCallback(HFftResults* result, size_t length) {

    // Store the current spectrum in the output buffer
    memcpy((void*) _audioSpectrum, (void*) result->Spectrum, sizeof(double) * _audioFftSize / 2);
    return length;
}

int BoomaApplication::GetAudioFftSize() {
    return _audioFftSize / 2;
}

int BoomaApplication::GetAudioSpectrum(double* spectrum) {
    memcpy((void*) spectrum, _audioSpectrum, sizeof(double) * _audioFftSize / 2);
    return _audioFftSize / 2;
}

bool BoomaApplication::SetFrequency(long int frequency) {
    if( _receiver->SetFrequency(frequency) ) {
        _opts->SetFrequency(frequency);
        return true;
    }
    return false;
}

long int BoomaApplication::GetFrequency() {
    return _opts->GetFrequency();
}

bool BoomaApplication::ChangeFrequency(int stepSize) {
    if( _receiver->SetFrequency(_opts->GetFrequency() + stepSize) ) {
        _opts->SetFrequency(_opts->GetFrequency() + stepSize);
        return true;
    }
    return false;
}

bool BoomaApplication::SetVolume(int volume) {
    if( volume >= 10 || volume <= 0 ) {
        return false;
    }
    _opts->SetVolume(volume);
    _outputWriter->SetGain(_opts->GetVolume());
    return true;
}

bool BoomaApplication::ChangeVolume(int stepSize) {
    if( _opts->GetVolume() + stepSize >= 10 || _opts->GetVolume() + stepSize <= 0 ) {
        return false;
    }
    _opts->SetVolume(_opts->GetVolume() + stepSize);
    _outputWriter->SetGain(_opts->GetVolume());
    return true;
}

int BoomaApplication::GetVolume() {
    return _opts->GetVolume();
}

bool BoomaApplication::ToggleDumpRf() {
    _opts->SetDumpRf( _input->SetDumpRf(!_opts->GetDumpRf()) );
    return true;
}

bool BoomaApplication::GetDumpRf() {
    return !_opts->GetDumpRf();
}

bool BoomaApplication::ToggleDumpAudio() {
    _opts->SetDumpAudio(!_opts->GetDumpAudio());
    _audioMute->SetMuted(!_opts->GetDumpAudio());
    return true;
}

bool BoomaApplication::GetDumpAudio() {
    return !_audioMute->GetMuted();
}

bool BoomaApplication::SetRfGain(int gain) {
    if( _receiver->SetRfGain(gain) ) {
        _opts->SetRfGain(gain);
        return true;
    }
    return false;
}

bool BoomaApplication::ChangeRfGain(int stepSize) {
    if( _receiver->SetRfGain(_opts->GetRfGain() + stepSize) ) {
        _opts->SetRfGain(_opts->GetRfGain() + stepSize);
        return true;
    }
    return false;
}

int BoomaApplication::GetRfGain() {
    return _opts->GetRfGain();
}
