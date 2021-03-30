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

class MainWindow {

    private:

        // Receiver
        BoomaApplication* _app;

        // Widgets
        Fl_Window *_win;
        Fl_Menu_Bar *_menubar;

        // Compose GUI
        void SetupMenus();

        // Utility methods
        char* GetTitle();

    public:

        // Construction/Destruction
        MainWindow(BoomaApplication* app);
        ~MainWindow();

        // Menubar handling
        void HandleMenuButton(char* name);

        // Exit
        void Exit();
};

#endif