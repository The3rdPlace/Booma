//
// Created by henrik on 2021-03-25.
//

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

        // Dynamic labels
        char _gainLabel[50];
        char _volumeLabel[50];

        // Display widgets
        Waterfall* _rfInputWaterfall;

        // Compose GUI
        void SetupMenus();
        void SetupControls();
        void SetupDisplays();

        // Helper functions for setting up the GUI
        void SetupFileMenu();
        void SetupConfigurationMenu();
        void SetupReceiverInputMenu();
        void SetupReceiverOutputMenu();
        void SetupReceiverInputFilterMenu();
        void SetupReceiverModeMenu();
        void SetupOptionsMenu();

        // Internal menu handling
        void HandleMenuButtonReceiverInput(char* name, char* value);
        void HandleMenuButtonReceiverOutput(char* name, char* value);
        void HandleMenuButtonReceiverMode(char* name, char* value);
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
        void HandleVolumeSlider();

    // Exit
        void Exit();
};

#endif