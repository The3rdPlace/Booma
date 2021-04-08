#include "inputdialog.h"

InputDialog::InputDialog(BoomaApplication* app, Mode mode):
    _app(app),
    _mode(mode) {
}

InputDialog::~InputDialog() {
    delete(_name);
    delete(_win);
}

void InputDialog::Show() {

    const int x = 10;
    const int ix = 150;
    const int rx = 180;
    int y = 0;
    int yi = 1;
    const int width = 30;
    const int fwidth = 120;
    const int height = 30;
    const int fheight = 25;

    // Create window
    _win = new Fl_Window(400, 600);
    _win->set_modal();
    _win->label(_mode == EDIT ? "Edit input" : "Add new input");

    // Add form fields
    _name = new Fl_Input(60, (y++ * height), fwidth * 2, fheight, "Name: ");
    _inputTypeGroup = new Fl_Group(10, (y++ * height), 300, 550);
    _inputTypeGroup->box(Fl_Boxtype::FL_DOWN_FRAME);
    _inputTypeGroup->color(22);

    _isAudioDevice = new Fl_Radio_Round_Button(x, (yi++ * height), width, height, "Audio device: ");
    _localAudioDevice = new Fl_Input(ix, ((yi -1) * height), fwidth, fheight);
    _isRtlsdrDevice = new Fl_Radio_Round_Button(x, (yi++ * height), width, height, "Rtlsdr device");
    _localRtlsdrDevice = new Fl_Input(ix, ((yi -1) * height), fwidth, fheight);
    _isPcmFile = new Fl_Radio_Round_Button(x, (yi++ * height), width, height, "PCM file");
    _localPcmFilename = new Fl_Input(ix, ((yi -1) * height), fwidth, fheight);
    _isWavFile = new Fl_Radio_Round_Button(x, (yi++ * height), width, height, "WAV file");
    _localWavFilename  = new Fl_Input(ix, ((yi -1) * height), fwidth, fheight);
    _isSilence = new Fl_Radio_Round_Button(x, (yi++ * height), width, height, "Silence");
    _isGenerator = new Fl_Radio_Round_Button(x, (yi++ * height), width, height, "Signal generator");
    _localGeneratorFrequency = new Fl_Input(ix, ((yi -1) * height), fwidth, fheight);
    _isRemote = new Fl_Radio_Round_Button(x, (yi++ * height), width, height, "Remote input");
    _remoteHost = new Fl_Input(ix, ((yi - 1) * height), fwidth, fheight);

    _remoteInputTypeGroup = new Fl_Group(x, (yi * height), 300, 250);
    _remoteDataPort = new Fl_Input(ix, (yi++ * height), fwidth, fheight, "Data port");
    _remoteCommandPort = new Fl_Input(ix, (yi++ * height), fwidth, fheight, "Command port");
    _isRemoteAudioDevice = new Fl_Radio_Round_Button(ix, (yi++ * height), width, height, "Audio device: ");
    _isRemoteAudioDevice->align(FL_ALIGN_LEFT);
    _isRemoteRtlsdrDevice = new Fl_Radio_Round_Button(ix, (yi++ * height), width, height, "Rtlsdr device: ");
    _isRemoteRtlsdrDevice->align(FL_ALIGN_LEFT);
    _isRemotePcmFile = new Fl_Radio_Round_Button(ix, (yi++ * height), width, height, "PCM file: ");
    _isRemotePcmFile->align(FL_ALIGN_LEFT);
    _isRemoteWavFile = new Fl_Radio_Round_Button(ix, (yi++ * height), width, height, "WAV file: ");
    _isRemoteWavFile->align(FL_ALIGN_LEFT);
    _isRemoteSilence = new Fl_Radio_Round_Button(ix, (yi++ * height), width, height, "Silence: ");
    _isRemoteSilence->align(FL_ALIGN_LEFT);
    _isRemoteGenerator = new Fl_Radio_Round_Button(ix, (yi++ * height), width, height, "Signal generator: ");
    _isRemoteGenerator->align(FL_ALIGN_LEFT);
    LoadState();

    // Group radio buttons

    _inputTypeGroup->add(_isAudioDevice);
    _inputTypeGroup->add(_localAudioDevice);
    _inputTypeGroup->add(_isRtlsdrDevice);
    _inputTypeGroup->add(_localRtlsdrDevice);
    _inputTypeGroup->add(_isPcmFile);
    _inputTypeGroup->add(_localPcmFilename);
    _inputTypeGroup->add(_isWavFile);
    _inputTypeGroup->add(_localWavFilename);
    _inputTypeGroup->add(_isSilence);
    _inputTypeGroup->add(_isGenerator);
    _inputTypeGroup->add(_localGeneratorFrequency);
    _inputTypeGroup->add(_isRemote);
    _inputTypeGroup->add(_remoteHost);

    _remoteInputTypeGroup->add(_remoteDataPort);
    _remoteInputTypeGroup->add((_remoteCommandPort));
    _remoteInputTypeGroup->add(_isRemoteAudioDevice);
    _remoteInputTypeGroup->add(_isRemoteRtlsdrDevice);
    _remoteInputTypeGroup->add(_isRemotePcmFile);
    _remoteInputTypeGroup->add(_isRemoteWavFile);
    _remoteInputTypeGroup->add(_isRemoteSilence);
    _remoteInputTypeGroup->add(_isRemoteGenerator);

    _remoteInputTypeGroup->end();
    _inputTypeGroup->end();


    // Done setting up the gui
    _win->end();
    _win->show();

    _remoteInputTypeGroup->clear_active();

    // Wait for the dialog to be closed
    while (_win->shown()) Fl::wait();
}

void InputDialog::LoadState() {

    // Name of this input configuration
    if( _mode == EDIT ) {
        _name->value(_app->GetConfigSection().c_str());
    } else {
        std::string newName;
        std::vector<std::string> sections = _app->GetConfigSections();
        for( int cnt = 0; ; cnt++ ) {
            newName = "new" + (cnt > 0 ? " " + std::to_string(cnt) : "");
            bool found = false;
            for (std::vector<std::string>::iterator it = sections.begin(); it != sections.end(); it++) {
                if( (*it) == newName ) {
                    found = true;
                    break;
                }
            }
            if( !found ) {
                break;
            }
        }
        _name->value(newName.c_str());
    }

    // Local or remote input ?
    switch(_app->GetInputSourceType()) {
        case NETWORK:
            _isRemote->setonly();
            break;
        case AUDIO_DEVICE:
            _isAudioDevice->setonly();
            break;
        case InputSourceType::RTLSDR:
            _isRtlsdrDevice->setonly();
            break;
        case InputSourceType::PCM_FILE:
            _isPcmFile->setonly();
            break;
        case InputSourceType::WAV_FILE:
            _isWavFile->setonly();
            break;
        case InputSourceType::SILENCE:
            _isSilence->setonly();
            break;
        case InputSourceType::SIGNAL_GENERATOR:
            _isGenerator->setonly();
            break;
        default:
            HError("Unknown input source type %d", _app->GetInputSourceType());
    }

    /*_remoteHost->clear_active();
    _remoteDataPort->clear_active();
    _remoteCommandPort->clear_active();*/
}

void InputDialog::UpdateState() {

}

void InputDialog::SaveState() {

}
