#include <hardt.h>
#include "main.h"
#include "booma.h"
#include "mainwindow.h"

MainWindow::MainWindow(BoomaApplication* app)
{
    SetTitle(app);
    set_default_size(600, 200);
}

void MainWindow::SetTitle(BoomaApplication* app) {
    std::stringstream ss;
    ss << "Booma " << BOOMAGUI_MAJORVERSION << "." << BOOMAGUI_MINORVERSION << "." << BOOMAGUI_BUILDNO;
    ss << "   (";
    ss << "libbooma " << BOOMA_MAJORVERSION << "." << BOOMA_MINORVERSION << "." << BOOMA_BUILDNO;
    ss << ", ";
    ss << " Hardt " << getVersion();
    ss << ")";
    set_title(ss.str());
}