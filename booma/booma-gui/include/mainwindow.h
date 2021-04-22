#ifndef BOOMA_MAINWINDOW_H
#define BOOMA_MAINWINDOW_H

#include "booma.h"
#include "boomaapplication.h"
#include "waterfall.h"

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Menu_Button.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Slider.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Output.H>
#include <FL/Fl_Light_Button.H>

class MainWindow {

    private:

        // Current instance
        static MainWindow* _instance;

        // Receiver
        BoomaApplication* _app;

        // Main windows and the menubar
        Fl_Window* _win;
        Fl_Menu_Bar* _menubar;

        // Control widgets
        Fl_Input* _frequencyInput;
        Fl_Button* _frequencyInputSet;
        Fl_Button* _frequencyInputUp100;
        Fl_Button* _frequencyInputDown100;
        Fl_Button* _frequencyInputUp1Khz;
        Fl_Button* _frequencyInputDown1Khz;
        Fl_Choice* _channelSelector;
        Fl_Slider* _gainSlider;
        Fl_Slider* _volumeSlider;
        Fl_Light_Button* _gainEnabled;

        // Statusbar
        Fl_Group* _statusbar;
        Fl_Output* _statusbarConfig;
        Fl_Output* _statusbarMode;
        Fl_Output* _statusbarHardwareFreq;
        Fl_Output* _statusbarRunningState;
        Fl_Output* _statusbarRecordingRf;
        Fl_Output* _statusbarRecordingAf;

        // Dynamic labels
        char _gainLabel[50];
        char _volumeLabel[50];

        // Display widgets
        Waterfall* _rfInputWaterfall;
        Fl_Slider* _signalLevelSlider;

        // Compose GUI
        void SetupMenus();
        void SetupControls();
        void SetupDisplays();

        // Helper functions for setting up the GUI
        void SetupFileMenu();
        void SetupConfigurationMenu();
        void SetupReceiverMenu();
        void SetupReceiverInputMenu();
        void SetupReceiverOutputMenu();
        void SetupReceiverModeMenu();
        void SetupSettingsMenu();
        void SetupStatusbar();

        // Internal menu handling
        void HandleMenuButtonReceiverStartStop();
        void HandleMenuButtonReceiverRestart();
        void HandleMenuButtonReceiverDumpRf();
        void HandleMenuButtonReceiverDumpAf();
        void HandleMenuButtonReceiverInput(char* name, char* value);
        void HandleMenuButtonReceiverOutput(char* name, char* value);
        void HandleMenuButtonReceiverMode(char* name, char* value);
        void HandleMenuButtonReceiverPreamp(char* name, char* value);
        void HandleMenuButtonReceiverOptions(char* name, char* value);
        void HandleMenuButtonConfigurationInputs(char* name, char* value);
        void HandleMenuButtonReceiverIfFilterWidth(char* name, char* value);

        // Configurations (inputs)
        void EditReceiverInput(const char* name);
        void AddReceiverInput();
        void DeleteReceiverInput(const char* name);

        // Utility methods
        char* GetTitle();
        void RemoveMenuSubItems(const char *name);
        void RenameMenuItem(const char* name, const char* newname);
        int MapFromGainSliderValue(long value);
        long MapToGainSliderValue(int value);
        void SetGainSliderLabel();
        void SetVolumeSliderLabel();
        void UpdateState();
        void UpdateStatusbar();

        // Display threads
        std::thread* _signalLevelThread;
        std::thread* _rfSpectrumThread;
        std::thread* _afSpectrumThread;
        bool _threadsRunning = true;
        bool _threadsPaused = false;
        inline void UpdateSignalLevelDisplay();
        inline void UpdateRfSpectrumDisplay();
        inline void UpdateAfSpectrumDisplay();

        // Halting and cleaning up
        void Dispose();

        void Run();
        void Halt();

    public:

        // Construction/Destruction
        MainWindow(BoomaApplication* app);
        ~MainWindow();

        // Get current instance
        static MainWindow* Instance() {
            return _instance;
        }

        // Callback handling
        void HandleMenuButton(char* name);
        void HandleFrequencyInputButtons(Fl_Widget *w);
        void HandleFrequencyInput(Fl_Widget *w);
        void HandleChannelSelector(Fl_Widget *w);
        void HandleGainSlider();
         void HandleGainEnabled();
        void HandleVolumeSlider();

        // Exit
        void Exit();
};

#endif