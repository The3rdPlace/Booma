#ifndef BOOMA_WATERFALL_H
#define BOOMA_WATERFALL_H

#include <FL/fl_draw.H>

class Waterfall : public Fl_Widget {

    private:

        void draw();

        int _n;
        Fl_Offscreen _ofscr;
        double* _fft;

    public:

        Waterfall(int X, int Y, int W, int H, const char *L, int n);
        ~Waterfall();

        void Refresh(double* fft);
};

#endif