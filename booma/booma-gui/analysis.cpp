#include "analysis.h"

#include <FL/Fl.H>
#include <iostream>
#include <math.h>

Analysis::Analysis(int X, int Y, int W, int H, const char *L, int n, int zoom, BoomaApplication* app)
    : Fl_Widget(X, Y, W, H, L),
    _n(n),
    _zoom(zoom),
    _app(app),
    _fft(nullptr),
    _averageSpectrum(nullptr) {

    _fft = new double[_n];
    memset((void*) _fft, 0, _n * sizeof(double));

    _hzPerBin =  ((((float) _app->GetOutputSampleRate() / (float) _zoom) / (float) 2 )) / ((float) _n);

    _averageSpectrum = new double[_n * 10];
    memset((void*) _averageSpectrum, 0, _n * sizeof(double) * 10);

    _xFactor = (float) w() / ((float) _n / 2);
    _yFactor = ((float) h() - (float) 30) / (float) 255;
}

Analysis::~Analysis() {
}

void Analysis::ReConfigure(int n, int zoom) {
    _n = n;
    _zoom = zoom;

    if( _fft != nullptr ) {
        delete _fft;
    }
    _fft = new double[_n];
    memset((void*) _fft, 0, _n * sizeof(double));

    _hzPerBin =  ((((float) _app->GetOutputSampleRate() / (float) _zoom) / (float) 2 )) / ((float) _n);

    if( _averageSpectrum != nullptr ) {
        delete _averageSpectrum;
    }
    _averageSpectrum = new double[_n * 10];
    memset((void*) _averageSpectrum, 0, _n * sizeof(double) * 10);

    _xFactor = (float) w() / ((float) _n / 2);
    _yFactor = ((float) h() - (float) 30) / (float) 255;
}

void Analysis::draw() {

    // Begin paining
    Fl_Offscreen _ofscr = fl_create_offscreen(w(), h());
    fl_begin_offscreen(_ofscr);

    switch( _type ) {
        case AVERAGE_SPECTRUM:
            DrawAverageSpectrum();
            break;
    }

    // Done, copy the updated waterfall to the screen
    fl_end_offscreen();
    fl_copy_offscreen(x(), y(), w(), h(), _ofscr, 0, 0);
    fl_delete_offscreen(_ofscr);
}

void Analysis::DrawAverageSpectrum() {

    // Background
    fl_rectf(0, 0, w(), h() - 23, FL_BLACK);
    fl_rectf(0, h() - 23, w(), 23, FL_GRAY);
    fl_rect(0, 0, w(), h(), FL_BLACK);

    // Spectrum
    double topI[_n / 2];
    int topIndex = -1;
    int maxI = -1;
    int maxX = -1;
    int maxC = -1;
    for( int i = 0; i < _n / 2; i++ ) {
        double avg;
        for( int j = 0; j < 10; j++ ) {
            avg += _averageSpectrum[(j * _n) + i];
        }
        avg /= 10;

        int x = ((float) i * _xFactor) + (_xFactor / (float) 2);
        int c= ColorMap(avg);
        int y = (float) c * _yFactor;
        fl_color(fl_rgb_color(c));
        fl_line(x, h() - 23, x, h() - 23 - y);

        if( c > maxC ) {
            maxI = i,
            maxX = x;
            maxC = c;
            topI[++topIndex] = i;
        }
    }

    // Most significant frequency
    if( maxI > 0 ) {
        int freq = (int) (((float) maxI * _hzPerBin) - (_hzPerBin / 2));
        std::string max = std::to_string(freq) +
                          "  (+/- " +
                          std::to_string((int) _hzPerBin / 2) +
                          ")";
        int width = fl_width(max.c_str()) / 2;
        fl_color(fl_rgb_color(0));
        if (maxX - width < 10) {
            fl_draw(max.c_str(), 10, h() - 6);
        } else if (maxX + width > w() - 10) {
            fl_draw(max.c_str(), w() - (2 * width), h() - 6);
        } else {
            fl_draw(max.c_str(), maxX - width, h() - 6);
        }

        // Drop down a marker from the spectrum
        fl_color(FL_BLACK);
        fl_line(maxX, h() - 22, maxX, h() - 18);

        // 2 Most dominant frequencies
        if( topIndex >= 2 ) {
            int first = (int) (((float) topI[topIndex] * _hzPerBin) - (_hzPerBin / (float) 2));
            int second = (int) (((float) topI[topIndex - 1] * _hzPerBin) - (_hzPerBin / (float) 2));

            std::string top = "";
            top += std::to_string(first);
            top += " <-> ";
            top += std::to_string(second);
            top += " = ";
            top += std::to_string(abs(first - second));

            fl_color(fl_rgb_color(90));
            if (maxX <= w() / 2) {
                fl_draw(top.c_str(), w() - (fl_width(top.c_str())) - 10, h() - 6);
            } else {
                fl_draw(top.c_str(), 10, h() - 6);
            }
        }
    }
}

void Analysis::Refresh() {
    static int skip = 0;

    memcpy((void*) &_averageSpectrum[skip * _n], (void*) _fft, _n * sizeof(double ));

    if( ++skip >= 10 ) {
        redraw();
        Fl::awake();

        skip = 0;
    }
}

int Analysis::handle(int event) {

    switch(event) {
        case FL_PUSH:
            return 1;
        case FL_RELEASE:
            return 1;
        default:
            return 0;
    }
}
