#ifndef LRI_CONTROL_PANEL_NFD_HELPER_H
#define LRI_CONTROL_PANEL_NFD_HELPER_H

#include "UI/Window.h"
#include <filesystem>

namespace LRI::RCI {
    struct FilePickResult {
        enum {
            SUCCESS, CANCELLED, ERR
        } state;

        std::filesystem::path outpath;
    };
    // This function is implemented in utils.cpp
    FilePickResult pickRCIFile(Window* owner);

    void nfdHelperInit();
    void nfdHelperCleanup();
}

#endif // LRI_CONTROL_PANEL_NFD_HELPER_H
