#ifndef LRI_CONTROL_PANEL_NFD_HELPER_H
#define LRI_CONTROL_PANEL_NFD_HELPER_H

#include "UI/Window.h"
#include <filesystem>

namespace LRI::RCI {
    struct FilePickResult {
        enum {
            WAITING, SUCCESS, CANCELLED, ERR
        } state;

        std::filesystem::path outpath;
    };
    // This function is implemented in utils.cpp
    FilePickResult filePicker(Window* owner);

    void nfdHelperInit();
}

#endif // LRI_CONTROL_PANEL_NFD_HELPER_H
