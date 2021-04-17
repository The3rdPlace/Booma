#include "waterfall.h"

#include <FL/Fl.H>
#include <iostream>
#include <math.h>

Waterfall::Waterfall(int X, int Y, int W, int H, const char *L, int n, bool iq, BoomaApplication* app)
    : Fl_Widget(X, Y, W, H, L),
    _n(n),
    _iq(iq),
    _gw(W),
    _gh(H - 22),
    _app(app),
    _ghMinusOne(H - 22 - 1),
    _secondScreenLineStart(_gw * 1 * 3),
    _oneScreenLineLength(_gw * 3),
    _fullScreenLengthMinusOne(_gw * (_gh - 1) * 3),
    _ghMinusThree(_gh - 3) {
    std::cout << "waterfall n=" << n << ", iq=" << iq << ", w=" << W << "\n";

    _fft = new double[_n];
    memset((void*) _fft, 0, _n * sizeof(double));

    _screen = new uchar[_gw * _gh * 3];
    memset((void*) _screen, 0, _gw * (_gh - 1) * 3);

    for(int i =0; i < 256; i++ ) {
        if( i < 30 ) {
            _colorMap[i] = 30;
        } else {
            _colorMap[i] = i;
        }
    }

    for( int i = 0; i < 9; i++ ) {
        _gridLines[i] = i * (_gw / 8);
    };

    // Not actually Hz per frequency bin, but more - Hz pr. pixel for the given spectrum size
    _hzPerBin = ((float) _app->GetOutputSampleRate() / (float) (_iq ? 1 : 2)) / (float) _gw;
    std::cout << "hzPerBin=" << _hzPerBin << "\n";
}

#define W w()
#define H h()
#define GW _gw
#define GH _gh
#define GH_MINUS_ONE _ghMinusOne
#define FULL_SCREEN_LENGT_MINUS_ONE _fullScreenLengthMinusOne
#define SECOND_SCREEN_LINE_START _secondScreenLineStart
#define ONE_SCREEN_LINE_LENGTH _oneScreenLineLength
#define GH_MINUS_THREE _ghMinusThree
#define GH_PLUS_FIFTEEN _gh + 15

Waterfall::~Waterfall() {
    delete[] _fft;
    delete[] _screen;
}

void Waterfall::draw() {

    // Move waterfall downwards
    _ofscr = fl_create_offscreen(W, H);
    fl_begin_offscreen(_ofscr);
    fl_draw_image(_screen, 0, 1, GW, GH_MINUS_ONE);
    memmove((void*) &_screen[SECOND_SCREEN_LINE_START], (void*) _screen, FULL_SCREEN_LENGT_MINUS_ONE);
    memset((void*) _screen, 0, ONE_SCREEN_LINE_LENGTH);

    // Draw new waterfall line
    fl_rectf(0, 0, GW, 1, FL_BLACK);
    uchar* s = _screen;
    uchar c;
    if( _iq ) {
        for (int i = 0; i < _n && i < _gw * 2; i += 2) {
            if( _fft[i] > _fft[i + 1] ) {
                c = colorMap(_fft[i]);
            } else {
                c = colorMap(_fft[i + 1]);
            }
            fl_color(fl_rgb_color(c));
            fl_point(i / 2, 0);
            *(s++) = c;
            *(s++) = c;
            *(s++) = c;
        }
    } else {
        for (int i = 0; i < _n / 2 && i < _gw; i++) {
            c = colorMap(_fft[i]);
            fl_color(fl_rgb_color(c));
            fl_point(i, 0);
            *(s++) = c;
            *(s++) = c;
            *(s++) = c;
        }
    }

    // Draw current center frequency lines
    int center = _iq
            ? _app->GetOutputSampleRate() / 2 / _hzPerBin
            :((float) _app->GetFrequency() / _hzPerBin);
    fl_color(FL_DARK_YELLOW);
    fl_line_style(FL_DOT, 1, 0);
    fl_line(center - 4, 0, center - 4, _gh);
    fl_line(center + 4, 0, center + 4, _gh);

    // Draw current if filter width
    if( _app->GetInputFilterWidth() > 0 ) {
        int left;
        int right;
        if( _app->GetFrequency() - (_app->GetInputFilterWidth() / 2) > 0) {
            left = ((float) (_app->GetFrequency() - (_app->GetInputFilterWidth() / 2)) / _hzPerBin);
        } else {
            left = 0;
        }
        if( _app->GetFrequency() + (_app->GetInputFilterWidth() / 2) < (_app->GetOutputSampleRate() / 2) ) {
            right = ((float) (_app->GetFrequency() + (_app->GetInputFilterWidth() / 2)) / _hzPerBin);
        } else {
            right = _app->GetOutputSampleRate() / 2;
        }
        fl_rectf(left, GH_MINUS_THREE, right - left, 3, FL_RED);
    }

    // Frequency markers
    fl_rectf(0, GH, GW, H, FL_GRAY);
    fl_color(FL_BLACK);
    fl_color(40);
    fl_line_style(FL_DASH, 1, 0);
    for( int i = 0; i < 8; i++ ) {
        fl_line(_gridLines[i], 0, _gridLines[i], GH);
    }

    // Frequency labels
    std::string m0("0");
    std::string m1 = std::to_string(1 * _app->GetOutputSampleRate() / 8000);
    std::string m2 = std::to_string(2 * _app->GetOutputSampleRate() / 8000);
    std::string m3 = std::to_string(3 * _app->GetOutputSampleRate() / 8000);
    std::string m4 = std::to_string(4 * _app->GetOutputSampleRate() / 8000);
    fl_draw(m0.c_str(), 5, GH_PLUS_FIFTEEN);
    fl_draw(m1.c_str(), _gridLines[2] - fl_width(m1.c_str())/2, GH_PLUS_FIFTEEN);
    fl_draw(m2.c_str(), _gridLines[4] - fl_width(m1.c_str())/2, GH_PLUS_FIFTEEN);
    fl_draw(m3.c_str(), _gridLines[6] - fl_width(m1.c_str())/2, GH_PLUS_FIFTEEN);
    fl_draw(m4.c_str(), _gridLines[8] - fl_width(m4.c_str()) - 5, GH_PLUS_FIFTEEN);

    // Outer frame
    fl_line_style(FL_SOLID, 2, 0);
    fl_rect(1, 1, w() - 1, h() - 1, FL_BLACK);

    // Done, copy the updated waterfall to the screen
    fl_end_offscreen();
    fl_copy_offscreen(x(), y(), w(), h(), _ofscr, 0, 0);
    fl_delete_offscreen(_ofscr);
}

void Waterfall::Refresh() {
    redraw();
    Fl::awake();
}

void Waterfall::ReConfigure(bool iq) {
    std::cout << "waterfall n=" << _n << ", iq=" << iq << "\n";
    _iq = iq;
    _hzPerBin = ((float) _app->GetOutputSampleRate() / (float) (iq ? 1 : 2)) / (float) _gw;
    std::cout << "hzPerBin=" << _hzPerBin << "\n";
}