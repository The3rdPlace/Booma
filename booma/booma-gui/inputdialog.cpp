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
    _mode(mode),
    _changed(false) {
    InputDialog::_instance = this;
}

InputDialog::~InputDialog() {
    delete(_name);
    delete(_win);
}

#define YINC ((y++ * height) + 10)
#define YMINUS (((y -1) * height) + 10)
#define Y ((y * height) + 10)

bool InputDialog::Show() {

    int x = 10;
    const int ix = 150;
    int y = 0;
    const int width = 30;
    const int fwidth = 160;
    const int height = 30;
    const int fheight = 25;

    // Create window  630
    _win = new Fl_Window(595, 500);
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
    x += 410;
    y = 0;
    _name = new Fl_Input(420, YINC, fwidth, fheight, "Name: ");

    // Sample type
    y++;
    _inputSourceDataTypeGroup = new Fl_Group(x, Y, 255, 120);
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

    // Sample rate for input- and output device
    y++;
    _deviceRate = new Fl_Input(x, YINC, fwidth, height, "Input rate");
    _outputRate = new Fl_Input(x, YINC, fwidth, height, "Output rate");

    y = 15;
    x -= 80;
    _cancelButton = new Fl_Button(x - 10, Y, 4 * width, height, "Cancel");
    _cancelButton->callback(HandleCancelButtonCallback);
    _saveButton = new Fl_Button(x + (4 * width), Y, 4 * width, height, (_mode == EDIT ? "Save" : "Create"));
    _saveButton->callback(HandleSaveButtonCallback);

    // Set initial data content and state
    LoadState();

    // Done setting up the gui
    _win->end();
    _win->show();

    // Wait for the dialog to be closed
    while (_win->shown()) Fl::wait();

    // Signal if the configuration was changed or not
    return _changed;
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
            _localAudioDevice->value(std::to_string(_app->GetInputDevice()).c_str());
            _localRtlsdrDevice->value(std::to_string(_app->GetInputDevice()).c_str());
            break;
        case InputSourceType::RTLSDR:
            _isRtlsdrDevice->setonly();
            _localAudioDevice->value(std::to_string(_app->GetInputDevice()).c_str());
            _localRtlsdrDevice->value(std::to_string(_app->GetInputDevice()).c_str());
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

    // Various settings
    _localPcmFilename->value(_app->GetPcmFile().c_str());
    _localWavFilename->value(_app->GetWavFile().c_str());
    _localGeneratorFrequency->value(std::to_string(_app->GetSignalGeneratorFrequency()).c_str());
    _remoteHost->value(_app->GetRemoteServer().c_str());
    _remoteDataPort->value(std::to_string(_app->GetRemoteDataPort()).c_str());
    _remoteCommandPort->value(std::to_string(_app->GetRemoteCommandPort()).c_str());
    _deviceRate->value(std::to_string(_app->GetInputSampleRate()).c_str());
    _outputRate->value(std::to_string(_app->GetOutputSampleRate()).c_str());

    // Original input type (if none is set, that's perfectly allright. We may not need it)
    switch(_app->GetOriginalInputSourceType()) {
        case AUDIO_DEVICE:
            _isOriginalAudioDevice->setonly();
            break;
        case RTLSDR:
            _isOriginalRtlsdrDevice->setonly();
            break;
        case PCM_FILE:
            _isOriginalPcmFile->setonly();
            break;
        case WAV_FILE:
            _isOriginalWavFile->setonly();
            break;
        case SILENCE:
            _isOriginalSilence->setonly();
            break;
        case SIGNAL_GENERATOR:
            _isOriginalGenerator->setonly();
            break;
        default:
            break;
    }

    // Input sample type
    switch(_app->GetInputSourceDataType()) {
        case REAL_INPUT_SOURCE_DATA_TYPE:
            _inputSourceDatatypeReal->setonly();
            break;
        case IQ_INPUT_SOURCE_DATA_TYPE:
            _inputSourceDatatypeIq->setonly();
            break;
        case I_INPUT_SOURCE_DATA_TYPE:
            _inputSourceDatatypeI->setonly();
            break;
        case Q_INPUT_SOURCE_DATA_TYPE:
            _inputSourceDatatypeQ->setonly();
            break;
        default:
            break;
    }

    // Update gui state
    UpdateState();
}

void InputDialog::UpdateState() {

    // Local or remote input ?
    if( _isRemote->value() == 1 ) {
        _remoteInputTypeGroup->set_active();
        _originalInputTypeGroup->set_active();
        _inputSourceDataTypeGroup->set_active();
        _remoteInputTypeGroup->redraw();
        _originalInputTypeGroup->redraw();
        _inputSourceDataTypeGroup->redraw();

        _localAudioDevice->clear_active();
        _localRtlsdrDevice->clear_active();
        _localPcmFilename->clear_active();
        _localWavFilename->clear_active();
        _localGeneratorFrequency->clear_active();
        _remoteHost->set_active();
        _localAudioDevice->redraw();
        _localRtlsdrDevice->redraw();
        _localPcmFilename->redraw();
        _localWavFilename->redraw();
        _localGeneratorFrequency->redraw();
        _remoteHost->redraw();
    }
    if( _isAudioDevice->value() == 1 ) {
        _remoteInputTypeGroup->clear_active();
        _originalInputTypeGroup->clear_active();
        _inputSourceDataTypeGroup->clear_active();
        _remoteInputTypeGroup->redraw();
        _originalInputTypeGroup->redraw();
        _inputSourceDataTypeGroup->redraw();

        _localAudioDevice->set_active();
        _localRtlsdrDevice->clear_active();
        _localPcmFilename->clear_active();
        _localWavFilename->clear_active();
        _localGeneratorFrequency->clear_active();
        _remoteHost->clear_active();
        _localAudioDevice->redraw();
        _localRtlsdrDevice->redraw();
        _localPcmFilename->redraw();
        _localWavFilename->redraw();
        _localGeneratorFrequency->redraw();
        _remoteHost->redraw();
    }
    if( _isRtlsdrDevice->value() == 1 ) {
        _remoteInputTypeGroup->clear_active();
        _originalInputTypeGroup->clear_active();
        _inputSourceDataTypeGroup->clear_active();
        _remoteInputTypeGroup->redraw();
        _originalInputTypeGroup->redraw();
        _inputSourceDataTypeGroup->redraw();

        _localAudioDevice->clear_active();
        _localRtlsdrDevice->set_active();
        _localPcmFilename->clear_active();
        _localWavFilename->clear_active();
        _localGeneratorFrequency->clear_active();
        _remoteHost->clear_active();
        _localAudioDevice->redraw();
        _localRtlsdrDevice->redraw();
        _localPcmFilename->redraw();
        _localWavFilename->redraw();
        _localGeneratorFrequency->redraw();
        _remoteHost->redraw();
    }
    if( _isPcmFile->value() == 1 ) {
        _remoteInputTypeGroup->clear_active();
        _originalInputTypeGroup->set_active();
        _inputSourceDataTypeGroup->set_active();
        _remoteInputTypeGroup->redraw();
        _originalInputTypeGroup->redraw();
        _inputSourceDataTypeGroup->redraw();

        _localAudioDevice->clear_active();
        _localRtlsdrDevice->clear_active();
        _localPcmFilename->set_active();
        _localWavFilename->clear_active();
        _localGeneratorFrequency->clear_active();
        _remoteHost->clear_active();
        _localAudioDevice->redraw();
        _localRtlsdrDevice->redraw();
        _localPcmFilename->redraw();
        _localWavFilename->redraw();
        _localGeneratorFrequency->redraw();
        _remoteHost->redraw();
    }
    if( _isWavFile->value() == 1 ) {
        _remoteInputTypeGroup->clear_active();
        _originalInputTypeGroup->set_active();
        _inputSourceDataTypeGroup->set_active();
        _remoteInputTypeGroup->redraw();
        _originalInputTypeGroup->redraw();
        _inputSourceDataTypeGroup->redraw();

        _localAudioDevice->clear_active();
        _localRtlsdrDevice->clear_active();
        _localPcmFilename->clear_active();
        _localWavFilename->set_active();
        _localGeneratorFrequency->clear_active();
        _remoteHost->clear_active();
        _localAudioDevice->redraw();
        _localRtlsdrDevice->redraw();
        _localPcmFilename->redraw();
        _localWavFilename->redraw();
        _localGeneratorFrequency->redraw();
        _remoteHost->redraw();
    }
    if( _isSilence->value() == 1 ) {
        _remoteInputTypeGroup->clear_active();
        _originalInputTypeGroup->clear_active();
        _inputSourceDataTypeGroup->clear_active();
        _remoteInputTypeGroup->redraw();
        _originalInputTypeGroup->redraw();
        _inputSourceDataTypeGroup->redraw();

        _localAudioDevice->clear_active();
        _localRtlsdrDevice->clear_active();
        _localPcmFilename->clear_active();
        _localWavFilename->clear_active();
        _localGeneratorFrequency->clear_active();
        _remoteHost->clear_active();
        _localAudioDevice->redraw();
        _localRtlsdrDevice->redraw();
        _localPcmFilename->redraw();
        _localWavFilename->redraw();
        _localGeneratorFrequency->redraw();
        _remoteHost->redraw();
    }
    if( _isGenerator->value() == 1 ) {
        _remoteInputTypeGroup->clear_active();
        _originalInputTypeGroup->clear_active();
        _inputSourceDataTypeGroup->clear_active();
        _remoteInputTypeGroup->redraw();
        _originalInputTypeGroup->redraw();
        _inputSourceDataTypeGroup->redraw();

        _localAudioDevice->clear_active();
        _localRtlsdrDevice->clear_active();
        _localPcmFilename->clear_active();
        _localWavFilename->clear_active();
        _localGeneratorFrequency->set_active();
        _remoteHost->clear_active();
        _localAudioDevice->redraw();
        _localRtlsdrDevice->redraw();
        _localPcmFilename->redraw();
        _localWavFilename->redraw();
        _localGeneratorFrequency->redraw();
        _remoteHost->redraw();
    }
}

void InputDialog::SaveState() {
    if( _mode == EDIT ) {

    } else {

    }
    _changed = true;
}

void InputDialog::Cancel() {
    _win->hide();
}

void InputDialog::Save() {
    SaveState();
    _win->hide();
}
