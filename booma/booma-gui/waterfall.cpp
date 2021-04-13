#include "waterfall.h"

#include <FL/Fl.H>
#include <iostream>
#include <math.h>

Waterfall::Waterfall(int X, int Y, int W, int H, const char *L, int n, BoomaApplication* app)
    : Fl_Widget(X, Y, W, H, L),
    _n(n),
    _gw(W),
    _gh(H - 22),
    _app(app) {

    _fft = new double[_n];
    memset((void*) _fft, 0, _n * sizeof(double));

    _screen = new uchar[_gw * (_gh - 1) * 3];
    memset((void*) _screen, 0, _gw * (_gh - 1) * 3);

    for(int i =0; i < 256; i++ ) {
        if( i < 30 ) {
            _colorMap[i] = 30;
        } else {
            _colorMap[i] = i;
        }
    }

    _hzPerBin = ((float) _app->GetOutputSampleRate() / (float) 2) / (float) _n;
}

Waterfall::~Waterfall() {
    delete[] _fft;
    delete[] _screen;
}

void Waterfall::draw() {

    // Move waterfall downwards
    _ofscr = fl_create_offscreen(w(), h());
    fl_begin_offscreen(_ofscr);
    fl_draw_image(_screen, 0, 1, _gw, (_gh - 1));
    memmove((void*) &_screen[_gw * 1 * 3], (void*) _screen, _gw * (_gh - 1) * 3);
    memset((void*) _screen, 0, _gw * 3);

    // Draw new waterfall line
    fl_rectf(0, 0, _gw, 1, FL_BLACK);
    for( int i = 0; i < _n; i++ ) {
        uchar c = colorMap(_fft[i]);
        fl_color(fl_rgb_color(c));
        fl_point(i, 0);
        _screen[i * 3] = _screen[(i * 3) + 1] = _screen[(i * 3) + 2] = c;
    }

    // Draw current center frequency lines
    int center = ((float) _app->GetFrequency() / _hzPerBin);
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
        fl_rectf(left, _gh - 3, right - left, 3, FL_RED);
    }

    // Frequency markers
    fl_rectf(0, _gh, _gw, h(), FL_GRAY);
    fl_color(FL_BLACK);
    fl_draw("0", 5, _gh + 15);
    fl_draw(std::to_string(_app->GetOutputSampleRate() / 2000).c_str(), _gw - 22, _gh + 15);
    fl_draw(std::to_string(_app->GetOutputSampleRate() / 4000).c_str(), (_gw / 2), _gh + 15);
    fl_draw(std::to_string(_app->GetOutputSampleRate() / 8000).c_str(), (_gw / 4), _gh + 15);
    fl_draw(std::to_string(3 * _app->GetOutputSampleRate() / 8000).c_str(), (3 * _gw / 4), _gh + 15);
    fl_color(40);
    fl_line_style(FL_DASH, 1, 0);
    fl_line((_gw / 2), 0, _gw / 2, _gh);
    fl_line((_gw / 4), 0, _gw / 4, _gh);
    fl_line((3 * _gw / 4), 0, 3 * _gw / 4, _gh);
    fl_line((1 * _gw / 8), 0, 1 * _gw / 8, _gh);
    fl_line((3 * _gw / 8), 0, 3 * _gw / 8, _gh);
    fl_line((5 * _gw / 8), 0, 5 * _gw / 8, _gh);
    fl_line((7 * _gw / 8), 0, 7 * _gw / 8, _gh);


    // Outer frame
    fl_line_style(FL_SOLID, 2, 0);
    fl_rect(1, 1, w() - 1, h() - 1, FL_BLACK);


    // Done, copy the updated waterfall to the screen
    fl_end_offscreen();
    //fl_copy_offscreen(x(), y(), _gw, _gh, _ofscr, 0, 0);
    fl_copy_offscreen(x(), y(), w(), h(), _ofscr, 0, 0);
    fl_delete_offscreen(_ofscr);

}

void Waterfall::Refresh() {
    redraw();
    Fl::awake();
}