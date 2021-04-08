#ifndef BOOMA_INPUTDIALOG_H
#define BOOMA_INPUTDIALOG_H

#include "booma.h"
#include "boomaapplication.h"
#include "waterfall.h"

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Radio_Round_Button.H>

class InputDialog {

    public:

        enum Mode {EDIT, ADD};

    private:

        static InputDialog* _instance;
        BoomaApplication* _app;

        Fl_Window* _win;

        Mode _mode;
        std::string _configuration;

        Fl_Input* _name;

        Fl_Group* _inputTypeGroup;
        Fl_Radio_Round_Button* _isAudioDevice;
        Fl_Input* _localAudioDevice;
        Fl_Radio_Round_Button* _isRtlsdrDevice;
        Fl_Input* _localRtlsdrDevice;
        Fl_Radio_Round_Button* _isPcmFile;
        Fl_Input* _localPcmFilename;
        Fl_Radio_Round_Button* _isWavFile;
        Fl_Input* _localWavFilename;
        Fl_Radio_Round_Button* _isSilence;
        Fl_Radio_Round_Button* _isGenerator;
        Fl_Input* _localGeneratorFrequency;
        Fl_Radio_Round_Button* _isRemote;
        Fl_Group* _remoteInputTypeGroup;
        Fl_Input* _remoteHost;
        Fl_Input* _remoteDataPort;
        Fl_Input* _remoteCommandPort;
        Fl_Group* _originalInputTypeGroup;
        Fl_Radio_Round_Button* _isOriginalAudioDevice;
        Fl_Radio_Round_Button* _isOriginalRtlsdrDevice;
        Fl_Radio_Round_Button* _isOriginalPcmFile;
        Fl_Radio_Round_Button* _isOriginalWavFile;
        Fl_Radio_Round_Button* _isOriginalSilence;
        Fl_Radio_Round_Button* _isOriginalGenerator;
        Fl_Group* _inputSourceDataTypeGroup;
        Fl_Radio_Round_Button* _inputSourceDatatypeReal;
        Fl_Radio_Round_Button* _inputSourceDatatypeIq;
        Fl_Radio_Round_Button* _inputSourceDatatypeI;
        Fl_Radio_Round_Button* _inputSourceDatatypeQ;
        Fl_Button* _cancelButton;
        Fl_Button* _saveButton;

        void LoadState();
        void SaveState();
        void UpdateState();

    public:

        InputDialog(BoomaApplication* app, Mode mode);
        ~InputDialog();

        void Show();

        static InputDialog* GetInstance() {
            return _instance;
        }

        void Cancel();
        void Save();

        void Update() {
            UpdateState();
        }
};


#endif //BOOMA_INPUTDIALOG_H
