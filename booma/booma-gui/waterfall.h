#ifndef BOOMA_WATERFALL_H
#define BOOMA_WATERFALL_H

#include <FL/fl_draw.H>

class Waterfall : public Fl_Widget {

    private:

        int _n;
        Fl_Offscreen _ofscr;
        double* _fft;
        uchar* _screen;

        int _gw;
        int _gh;

        int gw() {
            return _gw;
        }

        int hw() {
            return _gh;
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