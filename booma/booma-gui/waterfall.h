#ifndef BOOMA_WATERFALL_H
#define BOOMA_WATERFALL_H

#include <FL/fl_draw.H>

class Waterfall : public Fl_Widget {

    private:

        int _n;
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

        inline int colorMap(int value) {
            int k = value / 10 ;
            int j = k > 255 ? 255 : k;
            return _colorMap[j];
        }

    public:

        Waterfall(int X, int Y, int W, int H, const char *L, int n);
        ~Waterfall();

        void draw();

        double* GetFftBuffer() {
            return _fft;
        }

        void Refresh();
};

#endif