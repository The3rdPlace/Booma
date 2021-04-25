#include "waterfall.h"

#include <FL/Fl.H>
#include <iostream>
#include <math.h>

Waterfall::Waterfall(int X, int Y, int W, int H, const char *L, int n, bool iq, BoomaApplication* app, int zoom, int center, WaterfallType type)
    : Fl_Widget(X, Y, W, H, L),
    _n(n),
    _iq(iq),
    _type(type),
    _zoom(zoom),
    _center(center),
    _gw(W),
    _gh(H - 22),
    _app(app),
    _ghMinusOne(H - 22 - 1),
    _secondScreenLineStart(_gw * 1 * 3),
    _oneScreenLineLength(_gw * 3),
    _fullScreenLengthMinusOne(_gw * (_gh - 1) * 3),
    _ghMinusThree(_gh - 3),
    _selectedFrequency(_app->GetFrequency()),
    _enableDrawing(true),
    _cb(nullptr),
    _enableNavigation(false) {

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
    _hzPerBin = _app->GetInputSourceDataType() == REAL_INPUT_SOURCE_DATA_TYPE
            ? (((float) _app->GetOutputSampleRate() / (float) 2) / (float) _zoom) / (float) _gw
            : ((float) _app->GetOutputSampleRate() / (float) _zoom) / (float) _gw;

    // Only enable click/drag navigation for the RF spectrum
    if( type == RF ) {
        _enableNavigation = true;
    }
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

    if( !_enableDrawing ) {
        return;
    }

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
        for (int i = _n / 4; i < _n / 2 && i < _gw; i++) {
            c = colorMap(_fft[i]);
            fl_color(fl_rgb_color(c));
            fl_point(i - (i / 4), 0);
            *(s++) = c;
            *(s++) = c;
            *(s++) = c;
        }
        for (int i = 0; i < _n / 4 && i < _gw / 2; i++) {
            c = colorMap(_fft[i]);
            fl_color(fl_rgb_color(c));
            fl_point(i + (i / 4), 0);
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

    if( _type == RF ) {

        // Draw current center frequency lines
        int center = _iq
                     ? ((_app->GetOutputSampleRate() / 2) + (_app->GetOffset())) / _hzPerBin
                     : ((float) _app->GetFrequency()) / _hzPerBin;
        fl_color(FL_DARK_RED);
        fl_line_style(FL_SOLID, 1, 0);
        fl_line(center - 4, 0, center - 4, _gh);
        fl_line(center + 4, 0, center + 4, _gh);

        // Draw current if filter width
        int halfFilterWidth = _app->GetInputFilterWidth() / 2;
        int halfSampleRate = _app->GetOutputSampleRate() / 2;
        if (halfFilterWidth > 0) {
            int left;
            int right;
            if (_iq) {
                left = ((halfSampleRate - halfFilterWidth) + (_app->GetOffset())) / _hzPerBin;
                right = ((halfSampleRate + halfFilterWidth) + (_app->GetOffset())) / _hzPerBin;
            } else {
                if (_app->GetFrequency() - halfFilterWidth > 0) {
                    left = ((float) (_app->GetFrequency() - halfFilterWidth) / _hzPerBin);
                } else {
                    left = 0;
                }
                if (_app->GetFrequency() + halfFilterWidth < halfSampleRate) {
                    right = ((float) (_app->GetFrequency() + halfFilterWidth) / _hzPerBin);
                } else {
                    right = halfSampleRate;
                }
            }
            fl_rectf(left, GH_MINUS_THREE, right - left, 3, FL_RED);
        }
    }

    // Frequency markers
    fl_rectf(0, GH, GW, H, FL_GRAY);
    fl_color(FL_BLACK);

    // Frequency markers and labels
    std::string m0;
    std::string m1;
    std::string m2;
    std::string m3;
    std::string m4;
    if( _iq ) {
        int zero = (_app->GetOutputSampleRate() / 4000) / _zoom;
        int halfRate = (_app->GetOutputSampleRate() / 2000) / _zoom;
        int freqKhz = (_app->GetFrequency() - _app->GetOffset())/ 1000;
        std::string suffix = _app->GetFrequency() % 1000 != 0 ? "." + std::to_string(_app->GetFrequency() % 1000).substr(0, 1) : "";
        m0 = std::to_string((0 * zero) - halfRate + freqKhz) + suffix;
        m1 = std::to_string((1 * zero) - halfRate + freqKhz) + suffix;
        m2 = std::to_string((2 * zero) - halfRate + freqKhz) + suffix;
        m3 = std::to_string((3 * zero) - halfRate + freqKhz) + suffix;
        m4 = std::to_string((4 * zero) - halfRate + freqKhz) + suffix;
    } else {
        int eightRate = (_app->GetOutputSampleRate() / 8000) / _zoom;
        if( _zoom > 1 ) {
            int freqKhz = (_center / 1000);
            m0 = std::to_string(freqKhz - (3 * eightRate));
            m1 = "";
            m2 = std::to_string(freqKhz + (0 * eightRate));
            m3 = "";
            m4 = std::to_string(freqKhz + (3 * eightRate));
        } else {
            m0 = "0";
            m1 = std::to_string(1 * eightRate);
            m2 = std::to_string(2 * eightRate);
            m3 = std::to_string(3 * eightRate);
            m4 = std::to_string(4 * eightRate);
        }
    }

    // Display Labels
    fl_draw(m0.c_str(), 5, GH_PLUS_FIFTEEN);
    if( m1 != "" ) {
        fl_draw(m1.c_str(), _gridLines[2] - fl_width(m1.c_str()) / 2, GH_PLUS_FIFTEEN);
    }
    fl_draw(m2.c_str(), _gridLines[4] - fl_width(m2.c_str())/2, GH_PLUS_FIFTEEN);
    if( m3 != "" ) {
        fl_draw(m3.c_str(), _gridLines[6] - fl_width(m3.c_str()) / 2, GH_PLUS_FIFTEEN);
    }
    fl_draw(m4.c_str(), _gridLines[8] - fl_width(m4.c_str()) - 5, GH_PLUS_FIFTEEN);

    fl_line_style(FL_SOLID, 2, 0);
    for( int i = 0; i < 8; i++ ) {
        // Todo: make these configurable
        //fl_line(_gridLines[i], 0, _gridLines[i], GH);
        fl_line(_gridLines[i], _gh, _gridLines[i], _gh + 3);
    }

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

void Waterfall::ReConfigure(bool iq, int zoom) {
    _iq = iq;
    _zoom = zoom;

    _hzPerBin = !iq
                ? (((float) _app->GetOutputSampleRate() / (float) 2) / (float) _zoom) / (float) _gw
                : ((float) _app->GetOutputSampleRate() / (float) _zoom) / (float) _gw;
}

int Waterfall::handle(int event) {

    if( !_enableNavigation ) {
        return 1;
    }

    static int firstX;
    static int firstY;
    static int lastX;
    static int lastY;
    static bool isDrag = false;

    switch(event) {
        case FL_PUSH:
            firstX = lastX = Fl::event_x();
            firstY = lastY = Fl::event_y();
            _enableDrawing = false;
            return 1;
        case FL_RELEASE:

            // Calculate the final selected frequency
            if( isDrag ) {
                int diff = ((lastX - this->x()) - (firstX - this->x()));
                int diffFreq = (diff * _hzPerBin);
                _selectedFrequency -= diffFreq;
            } else {
                _selectedFrequency = (_app->GetInputSourceDataType() != REAL_INPUT_SOURCE_DATA_TYPE
                                      ? (_app->GetFrequency() - _app->GetOffset() - (_app->GetOutputSampleRate() / 2))
                                      : 0) +
                                     ((lastX - this->x()) * _hzPerBin);
            }

            // Round to nearest 100 Hz
            _selectedFrequency = (_selectedFrequency / 100) * 100;

            // Reset
            isDrag = false;
            _enableDrawing = true;

            // Notify the callback that the frequency should change
            if( _cb != nullptr ) {
                _cb(this);
            }

            // Handled
            return 1;

        case FL_DRAG: {
            lastX = Fl::event_x();
            lastY = Fl::event_y();

            if( _iq ) {
                isDrag = true;

                int diff = lastX - firstX + this->x();
                fl_rectf(this->x(), this->y(), GW, GH, FL_BLACK);
                fl_draw_image(_screen, diff, this->y(), GW, GH);

                // Draw current center frequency lines
                if (_type == RF) {
                    int center = ((_app->GetOutputSampleRate() / 2) + (_app->GetOffset())) / _hzPerBin;
                    center += this->x();
                    fl_color(FL_YELLOW);
                    fl_line_style(FL_SOLID, 1, 0);
                    fl_line(center - 4, this->y(), center - 4, this->h() + this->y());
                    fl_line(center + 4, this->y(), center + 4, this->h() + this->y());
                }
            }

            return 1;
        }
        default:
            return Fl_Widget::handle(event);
    }
}
