#ifndef BOOMA_WATERFALL_H
#define BOOMA_WATERFALL_H

#include "boomaapplication.h"

#include <FL/fl_draw.H>

enum WaterfallType { RF, AF };

class Waterfall : public Fl_Widget {

    private:

        bool _enableDrawing;
        bool _enableNavigation;
        bool _mouseInside;
        int _mouseX;
        int _mouseY;
        bool _scheduleScreenshot;
        int _scale;

        BoomaApplication* _app;
        float _hzPerBin;

        long _selectedFrequency;

        WaterfallType _type;
        int _n;
        bool _iq;
        int _zoom;
        int _center;
        Fl_Offscreen _ofscr;
        double* _fft;
        uchar* _screen;
        uchar* _moveBuffer;
        uchar _colorMap[256];
        Fl_Callback0* _cb;
        std::string _screenshotPrefix = "waterfall";
        std::string _screenshotSuffix = "";
        int _screenshotSeq = 1;

        int _gw;
        int _gh;

        int gw() {
            return _gw;
        }

        int hw() {
            return _gh;
        }

        inline int colorMap(double value) {
            long k = value / (long) 20 ;
            int j = k > 255 ? 255 : k;
            return _colorMap[j];
        }

        // Precalculated variables (for speed)
        int _oneScreenLineLength;
        int _gridLines[9];

        // Utility methods
        void MoveSpectrum(int distance);

    public:

        Waterfall(int X, int Y, int W, int H, const char *L, int n, bool iq, BoomaApplication* app, int zoom, int center, WaterfallType type, int scale = 1);
        ~Waterfall();

        void ReConfigure(bool iq, int n, int zoom, int center);

        void draw();

        int handle(int event);

        double* GetFftBuffer() {
            return _fft;
        }

        void Refresh();

        void callback(Fl_Callback0* cb) {
            _cb = cb;
        }

        long GetSelectedFrequency() {
            return _selectedFrequency;
        }

        void Screenshot();

        void SetScreenshotPrefix(std::string prefix) {
            _screenshotPrefix = prefix;
        }

        void WriteScreenshot(unsigned char* image);
};

#endif