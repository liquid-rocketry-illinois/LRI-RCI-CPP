#ifndef LRI_CONTROL_PANEL_HDFVIEWER_H
#define LRI_CONTROL_PANEL_HDFVIEWER_H

#include "Windowlet.h"

namespace LRI::RCI {
    class HDFPicker : public Windowlet {
    public:
        explicit HDFPicker(Window* owner);
        ~HDFPicker() override = default;

        void render() override;
    };
}

#endif // LRI_CONTROL_PANEL_HDFVIEWER_H
