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
        Fl_Input* _remoteHost;
        Fl_Input* _remoteDataPort;
        Fl_Input* _remoteCommandPort;
        Fl_Group* _remoteInputTypeGroup;
        Fl_Radio_Round_Button* _isRemoteAudioDevice;
        Fl_Radio_Round_Button* _isRemoteRtlsdrDevice;
        Fl_Radio_Round_Button* _isRemotePcmFile;
        Fl_Radio_Round_Button* _isRemoteWavFile;
        Fl_Radio_Round_Button* _isRemoteSilence;
        Fl_Radio_Round_Button* _isRemoteGenerator;

        void LoadState();
        void UpdateState();
        void SaveState();

    public:

        InputDialog(BoomaApplication* app, Mode mode);
        ~InputDialog();

        void Show();
};


#endif //BOOMA_INPUTDIALOG_H
