#ifndef ESTOPVIEWER_H
#define ESTOPVIEWER_H

#include "../WModule.h"

namespace LRI::RCI {
    // A simple window module which will send an E-STOP packet when pushed
    class EStopViewer : public WModule {
    public:
        explicit EStopViewer() = default;
        ~EStopViewer() override = default;

        // Overridden render
        void render() override;
    };
} // namespace LRI::RCI

#endif // ESTOPVIEWER_H
