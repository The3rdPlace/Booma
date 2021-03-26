#include <hardt.h>
#include "main.h"
#include "booma.h"
#include "mainwindow.h"

MainWindow::MainWindow()
{
    std::stringstream ss;
    ss << "Booma " << BOOMAGUI_MAJORVERSION << "." << BOOMAGUI_MINORVERSION << "." << BOOMAGUI_BUILDNO;
    ss << " (";
    ss << "libbooma " << BOOMA_MAJORVERSION << "." << BOOMA_MINORVERSION << "." << BOOMA_BUILDNO;
    ss << ", ";
    ss << " Hardt " << getversion();
    ss << ")";

    set_title(ss.str());
    set_default_size(600, 200);
}