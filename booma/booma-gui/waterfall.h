#ifndef BOOMA_WATERFALL_H
#define BOOMA_WATERFALL_H

#include <FL/fl_draw.H>

class Waterfall : public Fl_Widget {

    private:

        void draw();

    public:

        Waterfall(int X, int Y, int W, int H, const char *L)
            : Fl_Widget(X, Y, W, H, L) {

        }
};

#endif