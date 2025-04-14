#ifndef ESTOPVIEWER_H
#define ESTOPVIEWER_H

#include "WModule.h"

namespace LRI::RCI {
    // A simple window which will send an E-STOP packet when pushed
    class EStopViewer : public WModule {
        static int CLASSID;

        const int classid;
        const ImVec2 size;

    public:
        explicit EStopViewer();
        ~EStopViewer() override = default;

        // Overridden render
        void render() override;
    };
}

#endif //ESTOPVIEWER_H
