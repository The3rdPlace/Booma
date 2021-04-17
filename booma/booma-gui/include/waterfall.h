#ifndef BOOMA_WATERFALL_H
#define BOOMA_WATERFALL_H

#include "boomaapplication.h"

#include <FL/fl_draw.H>

class Waterfall : public Fl_Widget {

    private:

        BoomaApplication* _app;
        float _hzPerBin;

        int _n;
        bool _iq;
        int _zoom;
        Fl_Offscreen _ofscr;
        double* _fft;
        uchar* _screen;
        uchar _colorMap[256];

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
        int _ghMinusOne;
        int _oneScreenLineLength;
        int _secondScreenLineStart;
        int _fullScreenLengthMinusOne;
        int _ghMinusThree;
        int _gridLines[9];

    public:

        Waterfall(int X, int Y, int W, int H, const char *L, int n, bool iq, BoomaApplication* app, int zoom);
        ~Waterfall();

        void ReConfigure(bool iq, int zoom);

        void draw();

        double* GetFftBuffer() {
            return _fft;
        }

        void Refresh();
};

#endif