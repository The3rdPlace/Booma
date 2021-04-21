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
#include <FL/Fl_Choice.H>
#include <FL/Fl_Native_File_Chooser.H>
#include <FL/Fl_Check_Button.H>

class InputDialog {

    public:

        enum Mode {EDIT, ADD};

    private:

        static InputDialog* _instance;
        BoomaApplication* _app;

        Fl_Window* _win;

        Mode _mode;
        std::string _configuration;
        bool _changed;

        Fl_Input* _name;

        Fl_Group* _inputTypeGroup;
        Fl_Radio_Round_Button* _isAudioDevice;
        Fl_Choice* _localAudioDevice;
        Fl_Radio_Round_Button* _isRtlsdrDevice;
        Fl_Choice* _localRtlsdrDevice;
        Fl_Radio_Round_Button* _isPcmFile;
        Fl_Input* _localPcmFilename;
        Fl_Button* _selectPcmFilename;
        Fl_Radio_Round_Button* _isWavFile;
        Fl_Input* _localWavFilename;
        Fl_Button* _selectWavFilename;
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
        Fl_Choice* _deviceRate;
        Fl_Choice* _outputRate;
        Fl_Input* _converterFrequency;
        Fl_Input* _frequencyAdjust;

        int GetCardNumber(std::string key, const Fl_Menu_Item* item);

        void LoadState();
        void SaveState();
        void UpdateState();

        std::string SelectFile(std::string filter);

        std::vector<int> _deviceRates = {
                H_SAMPLE_RATE_44K1,
                H_SAMPLE_RATE_48K,
                H_SAMPLE_RATE_96K,
                H_SAMPLE_RATE_192K,
                H_SAMPLE_RATE_960K,
                H_SAMPLE_RATE_1M152K,
                H_SAMPLE_RATE_1M440K,
                H_SAMPLE_RATE_1M600K,
                H_SAMPLE_RATE_1M800K,
                H_SAMPLE_RATE_1M920K,
                H_SAMPLE_RATE_2M400K,
                H_SAMPLE_RATE_2M880K
        };

        std::vector<int> _outputRates = {
                H_SAMPLE_RATE_44K1,
                H_SAMPLE_RATE_48K,
                H_SAMPLE_RATE_96K
        };

    public:

        InputDialog(BoomaApplication* app, Mode mode);
        ~InputDialog();

        bool Show();

        static InputDialog* GetInstance() {
            return _instance;
        }

        void Cancel();
        void Save();

        void Update() {
            UpdateState();
        }

        void SelectPcmFile();
        void SelectWavFile();
};


#endif //BOOMA_INPUTDIALOG_H
