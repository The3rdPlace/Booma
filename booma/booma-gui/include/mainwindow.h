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
#include <FL/Fl_Value_Input.H>

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
        Fl_Input* _frequencyInput = nullptr;
        Fl_Button* _frequencyInputSet = nullptr;
        Fl_Button* _frequencyInputUp100 = nullptr;
        Fl_Button* _frequencyInputDown100 = nullptr;
        Fl_Button* _frequencyInputUp1Khz = nullptr;
        Fl_Button* _frequencyInputDown1Khz = nullptr;
        Fl_Input* _frequencyOffset = nullptr;
        Fl_Choice* _channelSelector = nullptr;
        Fl_Slider* _gainSlider = nullptr;
        Fl_Slider* _volumeSlider = nullptr;
        Fl_Light_Button* _gainEnabled = nullptr;

        // Statusbar
        Fl_Group* _statusbar = nullptr;
        Fl_Output* _statusbarConfig = nullptr;
        Fl_Output* _statusbarMode = nullptr;
        Fl_Output* _statusbarHardwareFreq = nullptr;
        Fl_Output* _statusbarRunningState = nullptr;
        Fl_Output* _statusbarRecordingRf = nullptr;
        Fl_Output* _statusbarRecordingAf = nullptr;

        // Dynamic labels
        char _gainLabel[50];
        char _volumeLabel[50];

        // Display widgets
        Waterfall* _rfInputWaterfall;
        Waterfall* _afOutputWaterfall;
        Fl_Slider* _signalLevelSlider;

        // Compose GUI
        void SetupMenus();
        void SetupControls();
        void SetupDisplays();

        // Helper functions for setting up the GUI
        void SetupChannels();
        void SetupFileMenu();
        void SetupConfigurationMenu();
        void SetupReceiverMenu();
        void SetupReceiverInputMenu();
        void SetupReceiverOutputMenu();
        void SetupReceiverModeMenu();
        void SetupSettingsMenu();
        void SetupStatusbar();
        void SetupNavigationMenu();

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
        std::thread* _halterThread;
        static bool _threadsRunning;
        static int _threadsAlive;
        bool _threadsPaused = false;
        inline void UpdateSignalLevelDisplay();
        inline void UpdateRfSpectrumDisplay();
        inline void UpdateAfSpectrumDisplay();

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
        void HandleFrequencyOffset(Fl_Widget *w);
        void HandleGainSlider();
        void HandleGainEnabled();
        void HandleVolumeSlider();
        void HandleRfWaterfall();
        void HandleFrequencyNavigation(char* name);
        void HandleCtrlF();
        void HandleCtrlO();
        void HandleEscape();

        // Exit
        void Exit();
};

#endif