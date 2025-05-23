#include "waterfall.h"

#include <FL/Fl.H>
#include <iostream>
#include <chrono>
#include <ctime>
#include <jpeglib.h>

Waterfall::Waterfall(int X, int Y, int W, int H, const char *L, int n, bool iq, BoomaApplication* app, int zoom, int center, WaterfallType type, int scale)
    : Fl_Widget(X, Y, W, H, L),
    _n(n),
    _iq(iq),
    _type(type),
    _zoom(zoom),
    _center(center),
    _gw(W),
    _gh(H - 22),
    _app(app),
    _oneScreenLineLength(_gw * 3),
    _selectedFrequency(_app->GetFrequency()),
    _enableDrawing(true),
    _cb(nullptr),
    _enableNavigation(false),
    _mouseInside(false),
    _scheduleScreenshot(false),
    _scale(scale) {

    _fft = new double[_n];
    memset((void*) _fft, 0, _n * sizeof(double));

    _screen = new uchar[_gw * _gh * 3];
    memset((void*) _screen, 0, _gw * (_gh - 1) * 3);

    _moveBuffer = new uchar[_gw * 3];

    for(int i =0; i < 256; i++ ) {
        if( i < 20 ) {
            _colorMap[i] = 0;
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

    // Set screenshot suffix
    std::time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    char tmp[256];
    std::strftime(tmp, 256, "%Y%m%d_%H%M%S", std::localtime(&now));
    _screenshotSuffix = tmp;
}

#define W w()
#define H h()
#define GW _gw
#define GH _gh
#define ONE_SCREEN_LINE_LENGTH _oneScreenLineLength
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
    fl_draw_image(_screen, 0, _scale, GW, GH - _scale);
    memmove((void*) &_screen[ONE_SCREEN_LINE_LENGTH * _scale], (void*) _screen, ONE_SCREEN_LINE_LENGTH * (GH - _scale));
    memset((void*) _screen, 0, ONE_SCREEN_LINE_LENGTH * _scale);

    // Draw new waterfall line
    fl_rectf(0, 0, GW, 1, FL_BLACK);
    uchar* s = _screen;
    uchar c;
    for( int j = 0; j < _scale; j++ ) {
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
    }

    if( _type == RF ) {

        if( _mouseInside ) {

            // Draw current center frequency lines
            int center = _iq
                         ? ((_app->GetOutputSampleRate() / 2) + (_app->GetOffset())) / _hzPerBin
                         : ((float) _app->GetFrequency()) / _hzPerBin;
            fl_color(FL_RED);
            fl_line_style(FL_SOLID, 2, 0);
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
                fl_rectf(left, GH - (_iq ? 6 : 3), right - left, 3, FL_RED);
            }

            // Draw current decimation filter width
            if (_iq) {
                int left = ((_app->GetOutputSampleRate() / 2) - (_app->GetDecimatorCutoff())) / _hzPerBin;
                int center = (_app->GetOutputSampleRate() / 2) / _hzPerBin;
                int right = ((_app->GetOutputSampleRate() / 2) + (_app->GetDecimatorCutoff())) / _hzPerBin;
                fl_color(FL_DARK_YELLOW);
                fl_line_style(FL_DASH, 2, 0);
                fl_line(center, 0, center, GH);
                fl_line_style(FL_SOLID, 3, 0);
                fl_line(left, GH - 10, left, GH);
                fl_line(right, GH - 10, right, GH);
                fl_rectf(left, GH - 3, right - left, 3, FL_DARK_YELLOW);
            }
        } else {

            // Draw current center frequency markers
            int center = _iq
                         ? ((_app->GetOutputSampleRate() / 2) + (_app->GetOffset())) / _hzPerBin
                         : ((float) _app->GetFrequency()) / _hzPerBin;
            fl_color(FL_RED);
            fl_line_style(FL_SOLID, 3, 0);
            fl_line(center - 1, 0, center - 1, 5);
            fl_line(center - 1, _gh, center - 1, _gh - 5);

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

    // Grid markers
    fl_line_style(FL_SOLID, 2, 0);
    for( int i = 0; i < 8; i++ ) {
        fl_line(_gridLines[i], _gh, _gridLines[i], _gh + 3);
    }

    // Outer frame
    fl_line_style(FL_SOLID, 2, 0);
    fl_rect(1, 1, w() - 1, h() - 1, FL_BLACK);

    // Mouse cursor
    if( _mouseInside ) {
        fl_line_style(FL_SOLID, 1, 0);
        fl_color(FL_GREEN);
        fl_line(_mouseX, 0, _mouseX, GH);

        long left = _iq
                ? _app->GetFrequency() - _app->GetOffset() - (_app->GetOutputSampleRate() / 2)
                : 0;
        std::string mouseFreq = std::to_string((int) ((_mouseX * _hzPerBin) + left));

        if( (_mouseX > (w() / 2) && _mouseX < (w() - (w() / 4))) || _mouseX < (w() / 4)) {
            fl_rectf(_mouseX + 5, 10, fl_width(mouseFreq.c_str()) + 10, 20, FL_BLACK);
            fl_color(FL_GREEN);
            fl_draw(mouseFreq.c_str(), _mouseX + 10, 25);
        } else {
            fl_rectf(_mouseX - 10 - fl_width(mouseFreq.c_str()), 10, fl_width(mouseFreq.c_str()) + 10, 20, FL_BLACK);
            fl_color(FL_GREEN);
            fl_draw(mouseFreq.c_str(), _mouseX - 5 - fl_width(mouseFreq.c_str()), 25);
        }
    }

    // Done, copy the updated waterfall to the screen
    fl_end_offscreen();
    fl_copy_offscreen(x(), y(), w(), h(), _ofscr, 0, 0);
    fl_delete_offscreen(_ofscr);

    // Take screenshot ?
    if( _scheduleScreenshot ) {
        uchar* buffer = new uchar[W * H * 3];
        fl_read_image(buffer, x(), y(), W, H);
        WriteScreenshot(buffer);
        delete[] buffer;
        _scheduleScreenshot = false;
        HLog("Screenshot completed");

        fl_color(FL_WHITE);
        fl_draw("SCREENSHOT COMPLETED", x() + 10, y() + 20);
    }
}

void Waterfall::WriteScreenshot(unsigned char* image) {

    std::string outname = "./" + _screenshotPrefix + "_" + _screenshotSuffix + "_" + std::to_string(_screenshotSeq++) + ".jpg";
    FILE* outfile = fopen(outname.c_str(), "wb");
    if (!outfile) {
        HError("Failed to open file for screenshot");
        return;
    }

    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr       jerr;

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);
    jpeg_stdio_dest(&cinfo, outfile);

    cinfo.image_width      = W;
    cinfo.image_height     = H;
    cinfo.input_components = 3;
    cinfo.in_color_space   = JCS_RGB;

    jpeg_set_defaults(&cinfo);
    jpeg_set_quality (&cinfo, 75, true);
    jpeg_start_compress(&cinfo, true);

    JSAMPROW row[1];

    while( cinfo.next_scanline < cinfo.image_height ) {
        int index = cinfo.next_scanline * 3 * _gw;
        row[0] = &image[index];
        jpeg_write_scanlines(&cinfo, row, 1);
    }

    jpeg_finish_compress(&cinfo);
    fclose(outfile);
}

void Waterfall::Refresh() {
    redraw();
    Fl::awake();
}

void Waterfall::ReConfigure(bool iq, int n, int zoom, int center) {
    _iq = iq;
    _n = n;
    _zoom = zoom;
    _center = center;

    _hzPerBin = !iq
                ? (((float) _app->GetOutputSampleRate() / (float) 2) / (float) _zoom) / (float) _gw
                : ((float) _app->GetOutputSampleRate() / (float) _zoom) / (float) _gw;

    if( _fft != nullptr ) {
        delete _fft;
    }
    _fft = new double[_n];
    memset((void*) _fft, 0, _n * sizeof(double));
}

int Waterfall::handle(int event) {

    if( !_enableNavigation ) {
        return 0;
    }

    static int firstX;
    static int firstY;
    static int lastX;
    static int lastY;
    static bool isDrag = false;

    switch(event) {
        case FL_PUSH:
            firstX = lastX = Fl::event_x() - x();
            firstY = lastY = Fl::event_y() - y();
            _enableDrawing = false;
            return 1;
        case FL_RELEASE:

            // Calculate the final selected frequency
            if( isDrag ) {
                int diff = ((lastX - this->x()) - (firstX - this->x()));
                int diffFreq = (diff * _hzPerBin);
                _selectedFrequency -= diffFreq;
                MoveSpectrum(lastX - firstX);
            } else {
                lastX = Fl::event_x() - x();
                lastY = Fl::event_y() - y();
                _selectedFrequency = (_app->GetInputSourceDataType() != REAL_INPUT_SOURCE_DATA_TYPE
                                      ? (_app->GetFrequency() - _app->GetOffset() - (_app->GetOutputSampleRate() / 2))
                                      : 0) +
                                     (lastX * _hzPerBin);
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
            lastX = Fl::event_x() - x();
            lastY = Fl::event_y() - y();

            if( _iq ) {
                isDrag = true;

                int diff = lastX - firstX + this->x();
                fl_rectf(this->x(), this->y(), GW, GH, FL_BLACK);
                fl_draw_image(_screen, diff, this->y(), GW, GH);

                // Draw current center frequency lines
                if (_type == RF) {
                    int center = ((_app->GetOutputSampleRate() / 2) + (_app->GetOffset())) / _hzPerBin;
                    center += this->x();
                    fl_color(FL_RED);
                    fl_line_style(FL_SOLID, 1, 0);
                    fl_line(center - 4, this->y(), center - 4, this->h() + this->y());
                    fl_line(center + 4, this->y(), center + 4, this->h() + this->y());
                }
            }

            return 1;
        }

        case FL_ENTER:
            _mouseInside = true;
            _mouseX = Fl::event_x() - x();
            _mouseY = Fl::event_y() - y();
            return 1;

        case FL_LEAVE:
            _mouseInside = false;
            return 1;

        case FL_MOVE:
            _mouseX = Fl::event_x() - x();
            _mouseY = Fl::event_y() - y();
            return 1;

        default:
            return 0;
    }
}

void Waterfall::MoveSpectrum(int distance) {

    if( distance == 0 ) {
        return;
    }

    if( distance < 0 ) {
        int startX = distance * -1;
        int remainingLength = _gw - startX;
        int fillStartX = _gw + distance;
        int fillLength = _gw - fillStartX;

        int lineStartPos = 0;

        startX *= 3;
        remainingLength *= 3;
        fillStartX *= 3;
        fillLength *= 3;

        for (int i = 0; i < _gh; i++) {
            memmove((void*) &_screen[lineStartPos], (void*) &_screen[lineStartPos + startX], remainingLength);
            memset((void*) &_screen[lineStartPos + fillStartX], 0, fillLength);
            lineStartPos += (_gw * 3);
        }
    } else {
        int startX = distance;
        int remainingLength = _gw - distance;
        int fillLength = distance;

        int lineStartPos = 0;

        startX *= 3;
        remainingLength *= 3;
        fillLength *= 3;

        for (int i = 0; i < _gh; i++) {
            memmove((void*) &_screen[lineStartPos + startX], (void*) &_screen[lineStartPos], remainingLength);
            memset((void*) &_screen[lineStartPos], 0, fillLength);
            lineStartPos += (_gw * 3);
        }
    }
}

void Waterfall::Screenshot() {
    _scheduleScreenshot = true;
    HLog("Screenshot scheduled");
}