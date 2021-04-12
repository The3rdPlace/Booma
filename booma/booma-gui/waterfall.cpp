#include "waterfall.h"

#include <FL/Fl.H>
#include <iostream>

Waterfall::Waterfall(int X, int Y, int W, int H, const char *L, int n)
    : Fl_Widget(X, Y, W, H, L),
    _n(n) {
    _fft = new double[_n];
}

Waterfall::~Waterfall() {
    delete[] _fft;
}

void Waterfall::draw() {
    /*fl_draw_box(FL_BORDER_BOX, x(), y(), w(), h(), FL_RED);
    fl_color(FL_BLACK);
    fl_draw("RF waterfall", 20, 60);
    fl_color(FL_WHITE);
    fl_rectf(40, 80, 120, 120);*/

    uchar* buffer = new uchar[120*120*3*4];
    fl_read_image(buffer, 40, 80, 120, 119);

    _ofscr = fl_create_offscreen(120, 120);
    fl_begin_offscreen(_ofscr);
//memset((void*) buffer, 55, sizeof(uchar) * 120*120*3);
    fl_draw_image(buffer, 0, 1, 120, 119);

    for( int i = 0; i < 100; i += 4) {
        fl_color(FL_RED);
        fl_point(0 + i, 0);
        fl_color(FL_GREEN);
        fl_point(1 + i + 2, 0);

    }


    fl_end_offscreen();
    fl_copy_offscreen(40, 80, 120, 120, _ofscr, 0, 0);

    fl_delete_offscreen(_ofscr);

    delete[] buffer;
    //fl_color(FL_WHITE);
    //fl_rectf(0, 0, 120, 120);
    std::cout << "draw" << std::endl;

}

void Waterfall::Refresh(double* fft) {
    memcpy((void*) _fft, (void*) fft, sizeof(double) * _n);
    /*std::cout << "refresh" << std::endl;
    Fl::lock();
    draw();
    Fl::unlock();
    Fl::awake();*/
/*
    _ofscr = fl_create_offscreen(120, 120);
    fl_begin_offscreen(_ofscr);

    fl_end_offscreen();
    fl_copy_offscreen(20, 20, 120, 120, _ofscr, 0, 0);

    fl_delete_offscreen(_ofscr);
*/
}