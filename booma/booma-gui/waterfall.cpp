#include "waterfall.h"

void Waterfall::draw() {
    fl_draw_box(FL_BORDER_BOX, x(), y(), w(), h(), FL_RED);
    fl_color(FL_BLACK);
    fl_draw("RF waterfall", 20, 60);

}
