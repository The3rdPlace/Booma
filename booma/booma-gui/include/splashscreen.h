#ifndef BOOMA_SPLASHSCREEN_H
#define BOOMA_SPLASHSCREEN_H

#include <iostream>
#include <vector>
#include <map>

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_PNG_Image.H>
#include <FL/fl_draw.H>

class SplashScreen: public Fl_Window {

    private:

        static SplashScreen* _instance;

        Fl_Button* _okButton;

        std::string _title;
        Fl_PNG_Image* _img;

    public:

        SplashScreen(int x, int y, std::string title);
        ~SplashScreen();

        static SplashScreen* GetInstance() {
            return _instance;
        }

        void draw();

        void Show();

        void Ok();
};

#endif