#ifndef LRI_CONTROL_PANEL_ERRORWINDOW_H
#define LRI_CONTROL_PANEL_ERRORWINDOW_H

#include <string>

#include "WModule.h"

namespace LRI::RCI {
    class ErrorWindow : public WModule {
        std::string errorMessage;

    public:
        ErrorWindow(const std::string& errorMessage) : errorMessage(errorMessage) {}
        ~ErrorWindow() override = default;

        void render() override;
    };
}

#endif // LRI_CONTROL_PANEL_ERRORWINDOW_H
