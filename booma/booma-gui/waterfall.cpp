#include "waterfall.h"

void Waterfall::draw() {
    fl_draw_box(FL_BORDER_BOX, x(), y(), w(), h(), FL_RED);
    fl_color(FL_BLACK);
    fl_draw("RF waterfall", 20, 60);

    char pixmap[100];
    for( int i = 0; i < 100; i += 4 ) {
        fl_point(20 + i, 100);
        fl_point(20 + i + 1, 100);
    }
Fl_Offscreen ofscr;

    
}
