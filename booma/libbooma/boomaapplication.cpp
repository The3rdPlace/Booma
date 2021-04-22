#include <include/boomaamreceiver.h>
#include "boomaapplication.h"
#include "boomacwreceiver.h"
#include "boomaauroralreceiver.h"
#include "boomassbreceiver.h"
#include "booma.h"

BoomaApplication::BoomaApplication(std::string appName, std::string appVersion, int argc, char** argv):
    _opts(NULL),
    _current(NULL),
    _input(NULL),
    _receiver(NULL),
    _output(NULL),
    _isRunning(false) {

    // Initialize the Hardt toolkit.
    HInit(std::string("Booma"), ConfigOptions::IsVerbose(argc, argv));

    // Show library name and and Hardt version.
    HLog("booma: using Hardt %s", getVersion().c_str());

    // Parse input arguments
    _opts = new ConfigOptions(appName, appVersion, argc, argv);

    // Initialize receiver
    if( !InitializeReceiver() ) {
        throw new BoomaReceiverException("Failed to create receiver, check the log");
    }
}

BoomaApplication::~BoomaApplication() {

    // Make sure that a running receiver has been shut down
    HLog("Shutting down a running receiver (should we have one)");
    Halt();

    // Delete the config object
    SyncConfiguration();
    HLog("Deleting the configuration object");
    if( _opts != NULL ) {
        delete _opts;
    }

    // Reset all previous receiver components
    HLog("Reset receiver components");
    if( _input != NULL ) {
        delete _input;
        _input = NULL;
    }
    if( _receiver != NULL ) {
        delete _receiver;
        _receiver = NULL;
    }
    if( _output != NULL ) {
        delete _output;
        _output = NULL;
    }
}

bool BoomaApplication::ChangeReceiver() {
    return ChangeReceiver(_opts->GetReceiverModeType());
}

bool BoomaApplication::Reconfigure() {

    // Make sure that a running receiver has been shut down
    HLog("Shutting down a running receiver (should we have one)");
    Halt();

    // Reset all previous receiver components
    if( _input != NULL ) {
        delete _input;
        _input = NULL;
    }
    if( _receiver != NULL ) {
        delete _receiver;
        _receiver = NULL;
    }
    if( _output != NULL ) {
        delete _output;
        _output = NULL;
    }

    // Configure new receiver
    HLog("Creating new receiver");
    if( !InitializeReceiver() ) {
        HError("Failed to create new receiver");
        return false;
    }
    return true;
}

bool BoomaApplication::ChangeReceiver(ReceiverModeType receiverModeType) {

    // Make sure we dont have any dump streams running
    if( _opts->GetDumpRf() ) {
        ToggleDumpRf();
    }
    if( _opts->GetDumpAudio() ) {
        ToggleDumpAudio();
    }

    // Register new receiver type
    HLog("Setting new receiver type");
    _opts->SetReceiverModeType(receiverModeType);

    // Reconfigure
    return Reconfigure();
}

bool BoomaApplication::InitializeReceiver() {

    try {

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
                _receiver = new BoomaCwReceiver(_opts, _input->GetIfFrequency());
                break;
            case AM:
                _receiver = new BoomaAmReceiver(_opts, _input->GetIfFrequency());
                break;
            case AURORAL:
                _receiver = new BoomaAuroralReceiver(_opts, _input->GetIfFrequency());
                break;
            case SSB:
                _receiver = new BoomaSsbReceiver(_opts, _input->GetIfFrequency());
                break;
            default:
                std::cout << "Unknown receiver type defined" << std::endl;
                return false;
        }

        // Build receiver
        if( !_receiver->IsFrequencySupported(_opts, _opts->GetFrequency()) ) {
            HLog("Unsupported frequency %d when starting new receiver. Using receivers default = %d", _opts->GetFrequency(), _receiver->GetDefaultFrequency(_opts));
            _opts->SetFrequency(_receiver->GetDefaultFrequency(_opts));
            HLog("Initial receiver frequency is now %d", _opts->GetFrequency());
        } else {
            HLog("Initial frequency %d is valid for the selected receiver", _opts->GetFrequency());
        }
        _receiver->Build(_opts, _input);

        // Setup output
        _output = new BoomaOutput(_opts, _receiver);

        // Set frequency - important when using a remote receiver
        SetFrequency(_opts->GetFrequency());
    }
    catch( BoomaException* e ) {
        HError("InitializeReceiver() Caught %s = %s", e->Type().c_str(), e->What().c_str() );
        return false;
    }

    // Complete input-receiver-output chain configured
    return true;
}

bool BoomaApplication::SetFrequency(long int frequency) {
    if( _input->SetFrequency(_opts, frequency) && _receiver->SetFrequency(_opts, _input->GetIfFrequency()) ) {
        _opts->SetFrequency(frequency);
        return true;
    }
    return false;
}

long int BoomaApplication::GetFrequency() {
    return _opts->GetFrequency();
}

bool BoomaApplication::ChangeFrequency(int stepSize) {
    return SetFrequency(_opts->GetFrequency() + stepSize);
}

bool BoomaApplication::SetVolume(int volume) {
    _opts->SetVolume( _output->SetVolume(volume) );
    return true;
}

bool BoomaApplication::ChangeVolume(int stepSize) {
    _opts->SetVolume( _output->SetVolume(_opts->GetVolume() + stepSize) );
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
    return _opts->GetDumpRf();
}

bool BoomaApplication::ToggleDumpAudio() {
    _opts->SetDumpAudio( _output->SetDumpAudio(!_opts->GetDumpAudio()) );
    return true;
}

bool BoomaApplication::GetDumpAudio() {
    return _opts->GetDumpAudio();
}

bool BoomaApplication::SetRfGain(int gain) {
    _opts->SetRfGain(gain);
    _receiver->SetRfGain(_opts->GetRfGain());
    return true;
}

bool BoomaApplication::ChangeRfGain(int stepSize) {
    return SetRfGain(_opts->GetRfGain() + stepSize);
}

int BoomaApplication::GetRfGain() {
    return _opts->GetRfGain();
}

int BoomaApplication::GetSignalLevel() {
    // We need to do our own S calculation since the agc must be be "removed" from the max value
    // The factor 25 is just picked from relative measurements.. this is not
    // a level that can be used for anything but a local reference!
    int max = GetSignalMax();
    return ((20 * log10((float) ceil((max == 0 ? 1 : max)))) / 6) - 4;
}

double BoomaApplication::GetSignalSum() {
    return _isRunning ? _output->GetSignalSum() : 0;
}

int BoomaApplication::GetSignalMax() {
    return _isRunning ? (_output->GetSignalMax() / _receiver->GetRfAgcCurrentGain()) : 0;
}

int BoomaApplication::GetRfFftSize() {
    return _input->GetRfFftSize();
}

int BoomaApplication::GetRfSpectrum(double* spectrum) {
    return _input->GetRfSpectrum(spectrum);
}

int BoomaApplication::GetAudioFftSize() {
    return _receiver->GetAudioFftSize();
}

int BoomaApplication::GetAudioSpectrum(double* spectrum) {
    return _receiver->GetAudioSpectrum(spectrum);
}

InputSourceType BoomaApplication::GetInputSourceType() {
    return _opts->GetInputSourceType();
}

bool BoomaApplication::SetInputSourceType(InputSourceType inputSourceType) {
    return _opts->SetInputSourceType(inputSourceType);
}

InputSourceType BoomaApplication::GetOriginalInputSourceType() {
    return _opts->GetOriginalInputSourceType();
}

bool BoomaApplication::SetOriginalInputSourceType(InputSourceType originalInputSourceType) {
    return _opts->SetOriginalInputSourceType(originalInputSourceType);
}

InputSourceDataType BoomaApplication::GetInputSourceDataType() {
    return _opts->GetInputSourceDataType();
}

bool BoomaApplication::SetInputSourceDataType(InputSourceDataType inputSourceDataType) {
    return _opts->SetInputSourceDataType(inputSourceDataType);
}

int BoomaApplication::GetInputDevice() {
    return _opts->GetInputDevice();
}

bool BoomaApplication::SetInputDevice(int device) {
    return _opts->SetInputDevice(device);
}

std::string BoomaApplication::GetPcmFile() {
    return _opts->GetPcmFile();
}

bool BoomaApplication::SetPcmFile(std::string filename) {
    return _opts->SetPcmFile(filename);
}

std::string BoomaApplication::GetWavFile() {
    return _opts->GetWavFile();
}

bool BoomaApplication::SetWavFile(std::string filename) {
    return _opts->SetWavFile(filename);
}

int BoomaApplication::GetSignalGeneratorFrequency() {
    return _opts->GetSignalGeneratorFrequency();
}

bool BoomaApplication::SetSignalGeneratorFrequency(int frequency) {
    return _opts->SetSignalGeneratorFrequency(frequency);
}

std::string BoomaApplication::GetRemoteServer() {
    return _opts->GetRemoteServer();
}

bool BoomaApplication::SetRemoteServer(std::string server) {
    return _opts->SetRemoteServer(server);
}

int BoomaApplication::GetRemoteDataPort() {
    return _opts->GetRemoteDataPort();
}

bool BoomaApplication::SetRemoteDataPort(int portnumber) {
    return _opts->SetRemoteDataPort(portnumber);
}

int BoomaApplication::GetRemoteCommandPort() {
    return _opts->GetRemoteCommandPort();
}

bool BoomaApplication::SetRemoteCommandPort(int portnumber) {
    return _opts->SetRemoteCommandPort(portnumber);
}

HTimer BoomaApplication::GetSchedule() {
    return _opts->GetSchedule();
}

bool BoomaApplication::GetEnableBuffers() {
    return _opts->GetEnableBuffers();
}

ReceiverModeType BoomaApplication::GetReceiver() {
    return _opts->GetReceiverModeType();
}

std::vector<Option>* BoomaApplication::GetOptions() {
    return _receiver->GetOptions();
}

int BoomaApplication::GetOption(std::string name) {
    return _receiver->GetOption(name);
}

bool BoomaApplication::SetOption(std::string name, std::string value) {
    return _receiver->SetOption(_opts, name, value);
}

std::string BoomaApplication::GetOptionInfoString() {
    return _receiver->GetOptionInfoString();
}

void BoomaApplication::SetBookmark(std::string name) {
    _opts->WriteBookmark(name);
}

void BoomaApplication::ApplyBookmark(std::string name) {
    if( _opts->ReadBookmark(name) ) {
        Reconfigure();
    }
}

void BoomaApplication::DeleteBookmark(std::string name) {
    _opts->DeleteBookmark(name);
}

std::vector<std::string> BoomaApplication::GetBookmarks() {
    return _opts->ListBookmarks();
}

std::map<int, Channel*> BoomaApplication::GetChannels() {
    return _opts->GetChannels();
}

bool BoomaApplication::AddChannel(std::string name, long int frequency) {
    std::string cleanName = name;
    std::replace(cleanName.begin(), cleanName.end(), ',', '.');
    std::replace(cleanName.begin(), cleanName.end(), ':', '.');
    return _opts->AddChannel(cleanName, frequency);
}

bool BoomaApplication::RemoveChannel(int id) {
    return _opts->RemoveChannel(id);
}

bool BoomaApplication::UseChannel(int id) {
    std::map<int, Channel*> channels = GetChannels();
    for( std::map<int, Channel*>::iterator it = channels.begin(); it != channels.end(); it++ ) {
        if( (*it).first == id ) {
            return SetFrequency((*it).second->Frequency);
        }
    }
    return false;
}

bool BoomaApplication::SetInputFilterWidth(int width) {

    _opts->SetInputFilterWidth(width);
    return _input->SetInputFilterWidth(_opts, width);
}

int BoomaApplication::GetInputFilterWidth() {
    return _opts->GetInputFilterWidth();
}

long BoomaApplication::GetShift() {
    if( _opts->GetInputSourceType() == InputSourceType::RTLSDR || _opts->GetOriginalInputSourceType() == InputSourceType::RTLSDR ) {
        return _opts->GetShift();
    } else {
        return 0;
    }
}

long BoomaApplication::GetRealShift() {
    return _opts->GetShift();
}

bool BoomaApplication::SetShift(long shift) {
    return _opts->SetShift(shift);
}

long BoomaApplication::GetFrequencyAdjust() {
    if( _opts->GetInputSourceType() == InputSourceType::RTLSDR || _opts->GetOriginalInputSourceType() == InputSourceType::RTLSDR ) {
        return _opts->GetRtlsdrAdjust();
    } else {
        return 0;
    }
}

long BoomaApplication::GetRealFrequencyAdjust() {
    return _opts->GetRtlsdrAdjust();
}

bool BoomaApplication::SetFrequencyAdjust(long adjust) {
    _opts->SetRtlsdrAdjust(adjust);
    return true;
}

std::vector<std::string> BoomaApplication::GetConfigSections() {
    return _opts->GetConfigSections();
}

std::string BoomaApplication::GetConfigSection() {
    return _opts->GetConfigSection();
}

bool BoomaApplication::SetConfigSection(std::string section) {
    if( _opts->SetConfigSection(section) ) {
        Reconfigure();
        return true;
    }
    return false;
}

bool BoomaApplication::CreateConfigSection(std::string section, bool cloneOldSettings, bool replaceDefault) {
    if( _opts->CreateConfigSection(section, cloneOldSettings, replaceDefault) ) {
        Reconfigure();
        return true;
    }
    return false;
}

bool BoomaApplication::RenameConfigSection(std::string newname) {
    if( _opts->RenameConfigSection(newname) ) {
        Reconfigure();
        return true;
    }
    return false;
}

bool BoomaApplication::DeleteConfigSection(std::string section) {
    return _opts->DeleteConfigSection(section);
}

int BoomaApplication::GetOutputFilterWidth() {
    return _receiver->GetOutputFilterWidth();
}

std::map<int, std::string> BoomaApplication::GetAudioDevices(bool hardwareDevices, bool virtualDevices, bool inputs, bool outputs) {
    return _opts->GetAudioDevices(hardwareDevices, virtualDevices, inputs, outputs);
}

std::map<int, std::string> BoomaApplication::GetRtlsdrDevices() {
    return _opts->GetRtlsdrDevices();
}

std::string BoomaApplication::GetOutputFilename() {
    return _opts->GetOutputFilename();
}

int BoomaApplication::GetOutputDevice() {
    return _opts->GetOutputAudioDevice();
}

void BoomaApplication::SetOutputAudioDevice(int card) {
    _opts->SetOutputAudioDevice(card);
    _opts->SetOutputFilename("");
    Reconfigure();
}

void BoomaApplication::SetOutputFilename(std::string filename) {
    _opts->SetOutputFilename(filename);
    _opts->SetOutputAudioDevice(-1);
    Reconfigure();
}

int BoomaApplication::GetInputSampleRate() {
    return _opts->GetInputSampleRate();
}

bool BoomaApplication::SetInputSampleRate(int rate) {
    return _opts->SetInputSampleRate(rate);
}

int BoomaApplication::GetOutputSampleRate() {
    return _opts->GetOutputSampleRate();
}

bool BoomaApplication::SetOutputSampleRate(int rate) {
    return _opts->SetOutputSampleRate(rate);
}

void BoomaApplication::SyncConfiguration() {
    _opts->SyncStoredConfig();
}

bool BoomaApplication::IsRunning() {
    return _isRunning;
}

bool BoomaApplication::SetPreampLevel(int level) {
    if( _input != nullptr && _input->SetPreampLevel(_opts, level) ) {
        _opts->SetPreamp(level);
        return true;
    }
    return false;
}

int BoomaApplication::GetPreampLevel() {
    return _opts->GetPreamp();
}

int BoomaApplication::GetOffset() {
    return _opts->GetRtlsdrOffset();
}

bool BoomaApplication::GetRfGainEnabled() {
    return _opts->GetRfGainEnabled();
}

bool BoomaApplication::SetRfGainEnabled(bool enabled) {
    if( _receiver->SetRfGainEnabled(enabled) ) {
        _opts->SetRfGainEnabled(enabled);
        return true;
    }
    return false;
}
