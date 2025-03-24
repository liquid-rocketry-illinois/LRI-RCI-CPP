#ifndef ESTOPVIEWER_H
#define ESTOPVIEWER_H

#include "BaseUI.h"

namespace LRI::RCI {

    // A simple window which will send an E-STOP packet when pushed
    class EStopViewer : public BaseUI {
        const ImVec2 size;

    public:
        explicit EStopViewer(const ImVec2&& size);
        ~EStopViewer() override = default;

        // Overridden render
        void render() override;
    };
}

#endif //ESTOPVIEWER_H
