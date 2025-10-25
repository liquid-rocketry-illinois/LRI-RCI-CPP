#ifndef LRI_CONTROL_PANEL_ERRORWINDOW_H
#define LRI_CONTROL_PANEL_ERRORWINDOW_H

#include <string>

#include "WModule.h"

namespace LRI::RCI {
    class ErrorWindow : public WModule {
    public:
        ErrorWindow() = default;
        ~ErrorWindow() override = default;

        void render() override;
    };
} // namespace LRI::RCI

#endif // LRI_CONTROL_PANEL_ERRORWINDOW_H
