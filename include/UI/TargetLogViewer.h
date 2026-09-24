#ifndef RAWVIEWER_H
#define RAWVIEWER_H

#include "Windowlet.h"

namespace LRI::RCI {
    // Window module which shows the output to the custom device class from RCP
    class TargetLogViewer : public WModule {
        bool autoscroll = true;

    public:
        explicit TargetLogViewer() = default;
        ~TargetLogViewer() override = default;

        // Overridden render function
        void render() override;
    };
} // namespace LRI::RCI

#endif // RAWVIEWER_H
