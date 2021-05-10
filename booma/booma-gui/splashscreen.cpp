#include "splashscreen.h"
#include "splash.h"
#include <hardt.h>

SplashScreen* SplashScreen::_instance;

void HandleSplashScreenOkButtonCallback(Fl_Widget* w, void* data) {
    SplashScreen::GetInstance()->Ok();
}

SplashScreen::SplashScreen(int x, int y, std::string title):
    Fl_Window(x, y, 420, 360),
    _title(title) {
    _instance = this;

    _img = new Fl_PNG_Image(NULL, Splash, 84050);
    if( _img->fail() == Fl_PNG_Image::ERR_FILE_ACCESS || _img->fail() == Fl_PNG_Image::ERR_FORMAT || _img->fail() == Fl_PNG_Image::ERR_NO_IMAGE ) {
        HError("Unable to load splash image: error is %d", _img->fail());
        delete _img;
        _img = nullptr;
    }
}

SplashScreen::~SplashScreen() {
    if( _img != nullptr ) {
        delete _img;
    }
}

void SplashScreen::Show() {
    set_modal();
    label(_title.c_str());

    begin();
    _okButton = new Fl_Button(310, 320, 100, 30, "&Ok");
    _okButton->callback(HandleSplashScreenOkButtonCallback);
    end();

    Fl_Window::show();

    // Wait for the dialog to be closed
    while (shown()) Fl::wait();
}

void SplashScreen::Ok() {
    hide();
}

void SplashScreen::draw() {
    Fl_Window::draw();

    if( _img != nullptr ) {
        _img->draw(10, 10);
        fl_draw("http://hardttoolkit.org/booma/", 10, 340);
    }
}
