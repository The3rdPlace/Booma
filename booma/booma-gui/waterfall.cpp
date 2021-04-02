#include "waterfall.h"

void Waterfall::draw() {
    fl_draw_box(FL_BORDER_BOX, x(), y(), w(), h(), FL_RED);
    fl_draw("hello", 300, 300);
    
}
