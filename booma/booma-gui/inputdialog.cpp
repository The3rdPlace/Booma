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

void HandleChoiceCallback(Fl_Widget* w, void* data) {
    InputDialog::GetInstance()->Update();
}

void HandleSelectPcmFileButtonCallback(Fl_Widget* w, void* data) {
    InputDialog::GetInstance()->SelectPcmFile();
}

void HandleSelectWavFileButtonCallback(Fl_Widget* w, void* data) {
    InputDialog::GetInstance()->SelectWavFile();
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
    _localAudioDevice = new Fl_Choice(ix, YMINUS, fwidth, fheight);
    _isRtlsdrDevice = new Fl_Radio_Round_Button(x, YINC, width, height, "Rtlsdr device");
    _localRtlsdrDevice = new Fl_Choice(ix, YMINUS, fwidth, fheight);
    _isPcmFile = new Fl_Radio_Round_Button(x, YINC, width, height, "PCM file");
    _localPcmFilename = new Fl_Input(ix, YMINUS, fwidth - 30, fheight);
    _selectPcmFilename = new Fl_Button(ix + fwidth - 30, YMINUS, 30, fheight, "...");
    _isWavFile = new Fl_Radio_Round_Button(x, YINC, width, height, "WAV file");
    _localWavFilename  = new Fl_Input(ix, YMINUS, fwidth - 30, fheight);
    _selectWavFilename = new Fl_Button(ix + fwidth - 30, YMINUS, 30, fheight, "...");
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
    _inputTypeGroup->add(_selectPcmFilename);
    _inputTypeGroup->add(_isWavFile);
    _inputTypeGroup->add(_localWavFilename);
    _inputTypeGroup->add(_selectWavFilename);
    _inputTypeGroup->add(_isSilence);
    _inputTypeGroup->add(_isGenerator);
    _inputTypeGroup->add(_localGeneratorFrequency);
    _inputTypeGroup->add(_isRemote);
    _inputTypeGroup->add(_remoteHost);
    _isAudioDevice->callback(HandleRadioButtonCallback);
    _localAudioDevice->callback(HandleChoiceCallback);
    _isRtlsdrDevice->callback(HandleRadioButtonCallback);
    _localRtlsdrDevice->callback(HandleChoiceCallback);
    _isPcmFile->callback(HandleRadioButtonCallback);
    _isWavFile->callback(HandleRadioButtonCallback);
    _isSilence->callback(HandleRadioButtonCallback);
    _isGenerator->callback(HandleRadioButtonCallback);
    _localGeneratorFrequency->callback(HandleChoiceCallback);
    _localGeneratorFrequency->when(FL_WHEN_CHANGED);
    _isRemote->callback(HandleRadioButtonCallback);
    _localPcmFilename->callback(HandleChoiceCallback);
    _localPcmFilename->when(FL_WHEN_CHANGED);
    _selectPcmFilename->callback(HandleSelectPcmFileButtonCallback);
    _localWavFilename->callback(HandleChoiceCallback);
    _localWavFilename->when(FL_WHEN_CHANGED);
    _selectWavFilename->callback(HandleSelectWavFileButtonCallback);
    _remoteHost->callback(HandleChoiceCallback);
    _remoteHost->when(FL_WHEN_CHANGED);
    _inputTypeGroup->end();

    // Settings for remote input
    _remoteInputTypeGroup = new Fl_Group(x, Y, 300, 60);
    _remoteDataPort = new Fl_Input(ix, YINC, fwidth, fheight, "Data port");
    _remoteCommandPort = new Fl_Input(ix, YINC, fwidth, fheight, "Command port");
    _remoteInputTypeGroup->add(_remoteDataPort);
    _remoteInputTypeGroup->add((_remoteCommandPort));
    _remoteDataPort->callback(HandleChoiceCallback);
    _remoteDataPort->when(FL_WHEN_CHANGED);
    _remoteCommandPort->callback(HandleChoiceCallback);
    _remoteCommandPort->when(FL_WHEN_CHANGED);
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
    _name->callback(HandleChoiceCallback);
    _name->when(FL_WHEN_CHANGED);

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
    _deviceRate = new Fl_Choice(x, YINC, fwidth, height, "Input rate");
    _outputRate = new Fl_Choice(x, YINC, fwidth, height, "Output rate");

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
        // Use name of current configuration
        _name->value(_app->GetConfigSection().c_str());
    } else {
        // Use a default name
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
    if( _mode == EDIT ) {
        switch (_app->GetInputSourceType()) {
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
    } else {
        _isSilence->setonly();
    }

    // Hardware input devices
    std::map<int, std::string> devices = _app->GetAudioDevices(true, true, true, false);
    for( std::map<int, std::string>::iterator it = devices.begin(); it != devices.end(); it++ ) {
        _localAudioDevice->add((*it).second.c_str());
        if( _mode == EDIT ) {
            if( (*it).first == _app->GetInputDevice() ) {
                _localAudioDevice->value((*it).first - 1);
            }
        }
    }
    devices = _app->GetRtlsdrDevices();
    for( std::map<int, std::string>::iterator it = devices.begin(); it != devices.end(); it++ ) {
        _localRtlsdrDevice->add((*it).second.c_str());
        if( _mode == EDIT ) {
            if( (*it).first == _app->GetInputDevice() ) {
                _localRtlsdrDevice->value((*it).first - 1);
            }
        }
    }

    // Various settings
    if( _mode == EDIT ) {
        _localPcmFilename->value(_app->GetPcmFile().c_str());
        _localWavFilename->value(_app->GetWavFile().c_str());
        _localGeneratorFrequency->value(std::to_string(_app->GetSignalGeneratorFrequency()).c_str());
        _remoteHost->value(_app->GetRemoteServer().c_str());
        _remoteDataPort->value(std::to_string(_app->GetRemoteDataPort()).c_str());
        _remoteCommandPort->value(std::to_string(_app->GetRemoteCommandPort()).c_str());
    }

    // Samplerates
    int i = 0;
    for( std::vector<int>::iterator it = _deviceRates.begin(); it != _deviceRates.end(); it++ ) {
        _deviceRate->add(std::to_string((*it)).c_str());
        if( _mode == EDIT ) {
            if( (*it) == _app->GetInputSampleRate() ) {
                _deviceRate->value(i);
            }
        } else {
            if( (*it) == 48000 ) {
                _deviceRate->value(i);
            }
        }
        i++;
    }
    i = 0;
    for( std::vector<int>::iterator it = _outputRates.begin(); it != _outputRates.end(); it++ ) {
        _outputRate->add(std::to_string((*it)).c_str());
        if( _mode == EDIT ) {
            if( (*it) == _app->GetOutputSampleRate() ) {
                _outputRate->value(i);
            }
        } else {
            if( (*it) == 48000 ) {
                _outputRate->value(i);
            }
        }
        i++;
    }

    // Original input type (if none is set, that's perfectly allright. We may not need it)
    if( _mode == EDIT ) {
        switch (_app->GetOriginalInputSourceType()) {
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
    } else {
        _isOriginalSilence->setonly();
    }

    // Input sample type
    if( _mode == EDIT ) {
        switch (_app->GetInputSourceDataType()) {
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
    } else {
        _inputSourceDatatypeReal->setonly();
    }

    // Update gui state
    UpdateState();
}

void InputDialog::UpdateState() {

    // Local or remote input ?
    if( _isRemote->value() == 1 ) {
        if( _remoteInputTypeGroup->active() == 0 ) {
            _remoteInputTypeGroup->set_active();
            _remoteInputTypeGroup->redraw();
        }
        if( _originalInputTypeGroup->active() == 0 ) {
            _originalInputTypeGroup->set_active();
            _originalInputTypeGroup->redraw();
        }

        _localAudioDevice->clear_active();
        _localRtlsdrDevice->clear_active();
        _localPcmFilename->clear_active();
        _selectPcmFilename->clear_active();
        _localWavFilename->clear_active();
        _selectWavFilename->clear_active();
        _localGeneratorFrequency->clear_active();
        _remoteHost->set_active();
        _localAudioDevice->redraw();
        _localRtlsdrDevice->redraw();
        _localPcmFilename->redraw();
        _selectPcmFilename->redraw();
        _localWavFilename->redraw();
        _selectWavFilename->redraw();
        _localGeneratorFrequency->redraw();
        _remoteHost->redraw();

        if( !std::string(_remoteHost->value()).empty() &&
            atoi(_remoteDataPort->value()) > 0 &&
            atoi(_remoteCommandPort->value() ) > 0) {
            _saveButton->set_active();
        } else {
            _saveButton->clear_active();
        }
        _saveButton->redraw();
    }
    if( _isAudioDevice->value() == 1 ) {
        _remoteInputTypeGroup->clear_active();
        _originalInputTypeGroup->clear_active();
        _remoteInputTypeGroup->redraw();
        _originalInputTypeGroup->redraw();

        _localAudioDevice->set_active();
        _localRtlsdrDevice->clear_active();
        _localPcmFilename->clear_active();
        _selectPcmFilename->clear_active();
        _localWavFilename->clear_active();
        _selectWavFilename->clear_active();
        _localGeneratorFrequency->clear_active();
        _remoteHost->clear_active();
        _localAudioDevice->redraw();
        _localRtlsdrDevice->redraw();
        _localPcmFilename->redraw();
        _selectPcmFilename->redraw();
        _localWavFilename->redraw();
        _selectWavFilename->redraw();
        _localGeneratorFrequency->redraw();
        _remoteHost->redraw();

        if( _localAudioDevice->value() >= 0 ) {
            _saveButton->set_active();
        } else {
            _saveButton->clear_active();
        }
        _saveButton->redraw();
    }
    if( _isRtlsdrDevice->value() == 1 ) {
        _remoteInputTypeGroup->clear_active();
        _originalInputTypeGroup->clear_active();
        _remoteInputTypeGroup->redraw();
        _originalInputTypeGroup->redraw();

        _localAudioDevice->clear_active();
        _localRtlsdrDevice->set_active();
        _localPcmFilename->clear_active();
        _selectPcmFilename->clear_active();
        _localWavFilename->clear_active();
        _selectWavFilename->clear_active();
        _localGeneratorFrequency->clear_active();
        _remoteHost->clear_active();
        _localAudioDevice->redraw();
        _localRtlsdrDevice->redraw();
        _localPcmFilename->redraw();
        _selectPcmFilename->redraw();
        _localWavFilename->redraw();
        _selectWavFilename->redraw();
        _localGeneratorFrequency->redraw();
        _remoteHost->redraw();

        if( _localRtlsdrDevice->value() >= 0 ) {
            _saveButton->set_active();
        } else {
            _saveButton->clear_active();
        }
        _saveButton->redraw();
    }
    if( _isPcmFile->value() == 1 ) {
        _remoteInputTypeGroup->clear_active();
        _remoteInputTypeGroup->redraw();
        if( _originalInputTypeGroup->active() == 0 ) {
            _originalInputTypeGroup->set_active();
            _originalInputTypeGroup->redraw();
        }

        _localAudioDevice->clear_active();
        _localRtlsdrDevice->clear_active();
        _localPcmFilename->set_active();
        _selectPcmFilename->set_active();
        _localWavFilename->clear_active();
        _selectWavFilename->clear_active();
        _localGeneratorFrequency->clear_active();
        _remoteHost->clear_active();
        _localAudioDevice->redraw();
        _localRtlsdrDevice->redraw();
        _localPcmFilename->redraw();
        _selectPcmFilename->redraw();
        _localWavFilename->redraw();
        _selectWavFilename->redraw();
        _localGeneratorFrequency->redraw();
        _remoteHost->redraw();

        if( !std::string(_localPcmFilename->value()).empty() ) {
            _saveButton->set_active();
        } else {
            _saveButton->clear_active();
        }
        _saveButton->redraw();
    }
    if( _isWavFile->value() == 1 ) {
        _remoteInputTypeGroup->clear_active();
        _remoteInputTypeGroup->redraw();
        if( _originalInputTypeGroup->active() == 0 ) {
            _originalInputTypeGroup->set_active();
            _originalInputTypeGroup->redraw();
        }

        _localAudioDevice->clear_active();
        _localRtlsdrDevice->clear_active();
        _localPcmFilename->clear_active();
        _selectPcmFilename->clear_active();
        _localWavFilename->set_active();
        _selectWavFilename->set_active();
        _localGeneratorFrequency->clear_active();
        _remoteHost->clear_active();
        _localAudioDevice->redraw();
        _localRtlsdrDevice->redraw();
        _localPcmFilename->redraw();
        _selectPcmFilename->redraw();
        _localWavFilename->redraw();
        _selectWavFilename->redraw();
        _localGeneratorFrequency->redraw();
        _remoteHost->redraw();

        if( !std::string(_localWavFilename->value()).empty() ) {
            _saveButton->set_active();
        } else {
            _saveButton->clear_active();
        }
        _saveButton->redraw();
    }
    if( _isSilence->value() == 1 ) {
        _remoteInputTypeGroup->clear_active();
        _originalInputTypeGroup->clear_active();
        _remoteInputTypeGroup->redraw();
        _originalInputTypeGroup->redraw();

        _localAudioDevice->clear_active();
        _localRtlsdrDevice->clear_active();
        _localPcmFilename->clear_active();
        _selectPcmFilename->clear_active();
        _localWavFilename->clear_active();
        _selectWavFilename->clear_active();
        _localGeneratorFrequency->clear_active();
        _remoteHost->clear_active();
        _localAudioDevice->redraw();
        _localRtlsdrDevice->redraw();
        _localPcmFilename->redraw();
        _selectPcmFilename->redraw();
        _localWavFilename->redraw();
        _selectWavFilename->redraw();
        _localGeneratorFrequency->redraw();
        _remoteHost->redraw();

        _saveButton->set_active();
        _saveButton->redraw();
    }
    if( _isGenerator->value() == 1 ) {
        _remoteInputTypeGroup->clear_active();
        _originalInputTypeGroup->clear_active();
        _remoteInputTypeGroup->redraw();
        _originalInputTypeGroup->redraw();

        _localAudioDevice->clear_active();
        _localRtlsdrDevice->clear_active();
        _localPcmFilename->clear_active();
        _selectPcmFilename->clear_active();
        _localWavFilename->clear_active();
        _selectWavFilename->clear_active();
        _localGeneratorFrequency->set_active();
        _remoteHost->clear_active();
        _localAudioDevice->redraw();
        _localRtlsdrDevice->redraw();
        _localPcmFilename->redraw();
        _selectPcmFilename->redraw();
        _localWavFilename->redraw();
        _selectWavFilename->redraw();
        _localGeneratorFrequency->redraw();
        _remoteHost->redraw();

        if( !std::string(_localGeneratorFrequency->value()).empty() &&
            atoi(_localGeneratorFrequency->value() ) > 0 ) {
            _saveButton->set_active();
        } else {
            _saveButton->clear_active();
        }
        _saveButton->redraw();
    }

    if( !std::string(_name->value()).empty() ) {
        _saveButton->set_active();
    } else {
        _saveButton->clear_active();
    }
    _saveButton->redraw();
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

void InputDialog::SelectPcmFile() {
    std::string file = SelectFile("PCM\t*.pcm");
    if( !file.empty() ) {
        _localPcmFilename->value(file.c_str());
        UpdateState();
    }
}

void InputDialog::SelectWavFile() {
    std::string file = SelectFile("WAV\t*.wav");
    if( !file.empty() ) {
        _localWavFilename->value(file.c_str());
        UpdateState();
    }
}

std::string InputDialog::SelectFile(std::string filter) {

    Fl_Native_File_Chooser chooser;
    chooser.title("Pick a file");
    chooser.type(Fl_Native_File_Chooser::BROWSE_FILE);
    chooser.filter(filter.c_str());
    chooser.directory(".");

    switch ( chooser.show() ) {
        case -1:
            HError("Filechooser error: %s\n", chooser.errmsg());
            return std::string();
        case 1:
            return std::string();
        default:
            return std::string(chooser.filename());
    }
}