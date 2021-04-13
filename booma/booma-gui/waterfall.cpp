#include "waterfall.h"

#include <FL/Fl.H>
#include <iostream>
#include <math.h>

Waterfall::Waterfall(int X, int Y, int W, int H, const char *L, int n, BoomaApplication* app)
    : Fl_Widget(X, Y, W, H, L),
    _n(n),
    _gw(W),
    _gh(H),
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
    _ofscr = fl_create_offscreen(_gw, _gh);
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

    // Done, copy the updated waterfall to the screen
    fl_end_offscreen();
    fl_copy_offscreen(x(), y(), _gw, _gh, _ofscr, 0, 0);
    fl_delete_offscreen(_ofscr);
}

void Waterfall::Refresh() {
    redraw();
    Fl::awake();
}