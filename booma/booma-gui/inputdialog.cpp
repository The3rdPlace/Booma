#include "inputdialog.h"

InputDialog* InputDialog::_instance;

void HandleCancelButtonCallback(Fl_Widget* w, void* data) {
    InputDialog::GetInstance()->Cancel();
}

void HandleSaveButtonCallback(Fl_Widget* w, void* data) {
    InputDialog::GetInstance()->Save();
}

void HandleRadioButtonCallback(Fl_Widget* w, void* data) {
    InputDialog::GetInstance()->Update();
}

InputDialog::InputDialog(BoomaApplication* app, Mode mode):
    _app(app),
    _mode(mode) {
    InputDialog::_instance = this;
}

InputDialog::~InputDialog() {
    delete(_name);
    delete(_win);
}

#define YINC ((y++ * height) + 10)
#define YMINUS (((y -1) * height) + 10)
#define Y ((y * height) + 10)

void InputDialog::Show() {

    int x = 10;
    const int ix = 150;
    int y = 0;
    const int width = 30;
    const int fwidth = 120;
    const int height = 30;
    const int fheight = 25;

    // Create window
    _win = new Fl_Window(630, 500);
    _win->set_modal();
    _win->label(_mode == EDIT ? "Edit input" : "Add new input");

    // Select input type
    _inputTypeGroup = new Fl_Group(10, Y, 300, 210);
    _isAudioDevice = new Fl_Radio_Round_Button(x, YINC, width, height, "Audio device");
    _localAudioDevice = new Fl_Input(ix, YMINUS, fwidth, fheight);
    _isRtlsdrDevice = new Fl_Radio_Round_Button(x, YINC, width, height, "Rtlsdr device");
    _localRtlsdrDevice = new Fl_Input(ix, YMINUS, fwidth, fheight);
    _isPcmFile = new Fl_Radio_Round_Button(x, YINC, width, height, "PCM file");
    _localPcmFilename = new Fl_Input(ix, YMINUS, fwidth, fheight);
    _isWavFile = new Fl_Radio_Round_Button(x, YINC, width, height, "WAV file");
    _localWavFilename  = new Fl_Input(ix, YMINUS, fwidth, fheight);
    _isSilence = new Fl_Radio_Round_Button(x, YINC, width, height, "Silence");
    _isGenerator = new Fl_Radio_Round_Button(x, YINC, width, height, "Signal generator");
    _localGeneratorFrequency = new Fl_Input(ix, YMINUS, fwidth, fheight);
    _isRemote = new Fl_Radio_Round_Button(x, YINC, width, height, "Remote input");
    _remoteHost = new Fl_Input(ix, YMINUS, fwidth, fheight);
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
    _isAudioDevice->callback(HandleRadioButtonCallback);
    _isRtlsdrDevice->callback(HandleRadioButtonCallback);
    _isPcmFile->callback(HandleRadioButtonCallback);
    _isWavFile->callback(HandleRadioButtonCallback);
    _isSilence->callback(HandleRadioButtonCallback);
    _isGenerator->callback(HandleRadioButtonCallback);
    _isRemote->callback(HandleRadioButtonCallback);
    _inputTypeGroup->end();

    // Settings for remote input
    _remoteInputTypeGroup = new Fl_Group(x, Y, 300, 60);
    _remoteDataPort = new Fl_Input(ix, YINC, fwidth, fheight, "Data port");
    _remoteCommandPort = new Fl_Input(ix, YINC, fwidth, fheight, "Command port");
    _remoteInputTypeGroup->add(_remoteDataPort);
    _remoteInputTypeGroup->add((_remoteCommandPort));
    _remoteInputTypeGroup->end();

    // Original input type - if needed
    y++;
    _originalInputTypeGroup = new Fl_Group(x, Y, 300, 180);
    _isOriginalAudioDevice = new Fl_Radio_Round_Button(x, YINC, width, height, "Original device: Audio device");
    _isOriginalRtlsdrDevice = new Fl_Radio_Round_Button(x, YINC, width, height, "Original device: Rtlsdr device");
    _isOriginalPcmFile = new Fl_Radio_Round_Button(x, YINC, width, height, "Original device: PCM file");
    _isOriginalWavFile = new Fl_Radio_Round_Button(x, YINC, width, height, "Original device: WAV file");
    _isOriginalSilence = new Fl_Radio_Round_Button(x, YINC, width, height, "Original device: Silence");
    _isOriginalGenerator = new Fl_Radio_Round_Button(x, YINC, width, height, "Original device: Signal generator");
    _originalInputTypeGroup->add(_isOriginalAudioDevice);
    _originalInputTypeGroup->add(_isOriginalRtlsdrDevice);
    _originalInputTypeGroup->add(_isOriginalPcmFile);
    _originalInputTypeGroup->add(_isOriginalWavFile);
    _originalInputTypeGroup->add(_isOriginalSilence);
    _originalInputTypeGroup->add(_isOriginalGenerator);
    _isOriginalAudioDevice->callback(HandleRadioButtonCallback);
    _isOriginalRtlsdrDevice->callback(HandleRadioButtonCallback);
    _isOriginalPcmFile->callback(HandleRadioButtonCallback);
    _isOriginalWavFile->callback(HandleRadioButtonCallback);
    _isOriginalSilence->callback(HandleRadioButtonCallback);
    _isOriginalGenerator->callback(HandleRadioButtonCallback);
    _originalInputTypeGroup->end();

    // Name of configuration
    x += 370;
    y = 0;
    _name = new Fl_Input(380, YINC, fwidth * 2, fheight, "Name: ");

    // Sample type
    y++;
    _inputSourceDataTypeGroup = new Fl_Group(x, Y, 290, 120);
    _inputSourceDatatypeReal = new Fl_Radio_Round_Button(x, YINC, width, height, "Realvalued samples");
    _inputSourceDatatypeIq = new Fl_Radio_Round_Button(x, YINC, width, height, "IQ samples");
    _inputSourceDatatypeI = new Fl_Radio_Round_Button(x, YINC, width, height, "I channel samples");
    _inputSourceDatatypeQ = new Fl_Radio_Round_Button(x, YINC, width, height, "Q channel samples");
    _inputSourceDataTypeGroup->add(_inputSourceDatatypeReal);
    _inputSourceDataTypeGroup->add(_inputSourceDatatypeIq);
    _inputSourceDataTypeGroup->add(_inputSourceDatatypeI);
    _inputSourceDataTypeGroup->add(_inputSourceDatatypeQ);
    _inputSourceDatatypeReal->callback(HandleRadioButtonCallback);
    _inputSourceDatatypeIq->callback(HandleRadioButtonCallback);
    _inputSourceDatatypeI->callback(HandleRadioButtonCallback);
    _inputSourceDatatypeQ->callback(HandleRadioButtonCallback);
    _inputSourceDataTypeGroup->end();

    y = 15;
    _cancelButton = new Fl_Button(x - 10, Y, 4 * width, height, "Cancel");
    _cancelButton->callback(HandleCancelButtonCallback);
    _saveButton = new Fl_Button(x + (4 * width), Y, 4 * width, height, (_mode == EDIT ? "Save" : "Create"));
    _saveButton->callback(HandleSaveButtonCallback);

    // Set initial data content and state
    LoadState();
    _remoteInputTypeGroup->set_active();

    // Done setting up the gui
    _win->end();
    _win->show();

    _originalInputTypeGroup->clear_active();
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
            _remoteInputTypeGroup->set_active();
            _originalInputTypeGroup->set_active();
            break;
        case AUDIO_DEVICE:
            _isAudioDevice->setonly();
            _remoteInputTypeGroup->clear_active();
            _originalInputTypeGroup->clear_active();
            break;
        case InputSourceType::RTLSDR:
            _isRtlsdrDevice->setonly();
            _remoteInputTypeGroup->clear_active();
            _originalInputTypeGroup->clear_active();
            break;
        case InputSourceType::PCM_FILE:
            _isPcmFile->setonly();
            _remoteInputTypeGroup->clear_active();
            _originalInputTypeGroup->set_active();
            break;
        case InputSourceType::WAV_FILE:
            _isWavFile->setonly();
            _remoteInputTypeGroup->clear_active();
            _originalInputTypeGroup->set_active();
            break;
        case InputSourceType::SILENCE:
            _isSilence->setonly();
            _remoteInputTypeGroup->clear_active();
            _originalInputTypeGroup->clear_active();
            break;
        case InputSourceType::SIGNAL_GENERATOR:
            _isGenerator->setonly();
            _remoteInputTypeGroup->clear_active();
            _originalInputTypeGroup->clear_active();
            break;
        default:
            HError("Unknown input source type %d", _app->GetInputSourceType());
    }
}

void InputDialog::UpdateState() {
std::cout << "UPDATESTATE()" << std::endl;
std::cout << "isremote = " << (_isRemote->value() == 0 ? "0" : "1") << std::endl;
    // Local or remote input ?
    if( _isRemote->value() == 1 ) {
        std::cout << "HERE"  << std::endl;
        _remoteInputTypeGroup->set_active();
        _originalInputTypeGroup->set_active();
        _inputSourceDataTypeGroup->set_active();
    }
    if( _isAudioDevice->value() == 1 ) {
        _remoteInputTypeGroup->clear_active();
        _originalInputTypeGroup->clear_active();
        _inputSourceDataTypeGroup->clear_active();
    }
    if( _isRtlsdrDevice->value() == 1 ) {
        _remoteInputTypeGroup->clear_active();
        _originalInputTypeGroup->clear_active();
        _inputSourceDataTypeGroup->clear_active();
    }
    if( _isPcmFile->value() == 1 ) {
        _remoteInputTypeGroup->clear_active();
        _originalInputTypeGroup->set_active();
        _inputSourceDataTypeGroup->set_active();
    }
    if( _isWavFile->value() == 1 ) {
        _remoteInputTypeGroup->clear_active();
        _originalInputTypeGroup->set_active();
        _inputSourceDataTypeGroup->set_active();
    }
    if( _isSilence->value() == 1 ) {
        _remoteInputTypeGroup->clear_active();
        _originalInputTypeGroup->clear_active();
        _inputSourceDataTypeGroup->clear_active();
    }
    if( _isGenerator->value() == 1 ) {
        _remoteInputTypeGroup->clear_active();
        _originalInputTypeGroup->clear_active();
        _inputSourceDataTypeGroup->clear_active();
    }
}

void InputDialog::SaveState() {

}

void InputDialog::Cancel() {
    _win->hide();
}

void InputDialog::Save() {
    if( _mode == EDIT ) {

    } else {

    }
    _win->hide();
}
