//
// Created by henrik on 2021-03-25.
//

#ifndef BOOMA_MAINWINDOW_H
#define BOOMA_MAINWINDOW_H

#include "booma.h"
#include "boomaapplication.h"

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Menu_Button.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Choice.H>

class MainWindow {

    private:

        // Current instance
        static MainWindow* _instance;

        // Receiver
        BoomaApplication* _app;

        // Widgets
        Fl_Window* _win;
        Fl_Menu_Bar* _menubar;
        Fl_Input* _frequencyInput;
        Fl_Button* _frequencyInputSet;
        Fl_Button* _frequencyInputUp100;
        Fl_Button* _frequencyInputDown100;
        Fl_Button* _frequencyInputUp1Khz;
        Fl_Button* _frequencyInputDown1Khz;
        Fl_Choice* _channelSelector;

        // Compose GUI
        void SetupMenus();
        void SetupControls();

        // Utility methods
        char* GetTitle();

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


    // Exit
        void Exit();
};

#endif