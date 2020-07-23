#include "application.h"
#include "cwreceiver.h"
#include "cwreceiver2.h"
#include "auroralreceiver.h"
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
                _receiver = new BoomaCwReceiver(_opts);
                break;
            case CW2:
                _receiver = new BoomaCwReceiver2(_opts);
                break;
            case AURORAL:
                _receiver = new BoomaAuroralReceiver(_opts);
                break;
            default:
                std::cout << "Unknown receiver type defined" << std::endl;
                return false;
        }
        _receiver->Build(_opts, _input);

        // Setup output
        _output = new BoomaOutput(_opts, _receiver);
    }
    catch( BoomaException* e ) {
        HError("InitializeReceiver() Caught %s = %s", e->Type().c_str(), e->What().c_str() );
        return false;
    }

    // Complete input-receiver-output chain configured
    return true;
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

int BoomaApplication::GetSignalLevel() {
    return _output->GetSignalLevel();
}

double BoomaApplication::GetSignalSum() {
    return _output->GetSignalSum();
}

int BoomaApplication::GetRfFftSize() {
    return _output->GetRfFftSize();
}

int BoomaApplication::GetRfSpectrum(double* spectrum) {
    return _output->GetRfSpectrum(spectrum);
}

int BoomaApplication::GetAudioFftSize() {
    return _output->GetAudioFftSize();
}

int BoomaApplication::GetAudioSpectrum(double* spectrum) {
    return _output->GetAudioSpectrum(spectrum);
}

InputSourceType BoomaApplication::GetInputSourceType() {
    return _opts->GetInputSourceType();
}

int BoomaApplication::GetInputAudioDevice() {
    return _opts->GetInputAudioDevice();
}

std::string BoomaApplication::GetPcmFile() {
    return _opts->GetPcmFile();
}

std::string BoomaApplication::GetWavFile() {
    return _opts->GetWavFile();
}

int BoomaApplication::GetSignalGeneratorFrequency() {
    return _opts->GetSignalGeneratorFrequency();
}

std::string BoomaApplication::GetRemoteServer() {
    return _opts->GetRemoteServer();
}

int BoomaApplication::GetRemoteDataPort() {
    return _opts->GetRemoteDataPort();
}

int BoomaApplication::GetRemoteCommandPort() {
    return _opts->GetRemoteDataPort();
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