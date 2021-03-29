//
// Created by henrik on 2021-03-25.
//

#ifndef BOOMA_MAINWINDOW_H
#define BOOMA_MAINWINDOW_H

#include <gtkmm.h>
#include <gtkmm/button.h>
#include <gtkmm/window.h>

#include "booma.h"
#include "boomaapplication.h"

class MainWindow : public Gtk::Window
{
    private:

        void SetTitle(BoomaApplication* app);

    public:
        MainWindow(BoomaApplication* app);
};

#endif