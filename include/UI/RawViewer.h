#ifndef RAWVIEWER_H
#define RAWVIEWER_H

#include "WModule.h"

namespace LRI::RCI {
    // Window module which shows the output to the custom device class from RCP
    class RawViewer : public WModule {

    public:
        explicit RawViewer() = default;
        ~RawViewer() override = default;

        // Overridden render function
        void render() override;
    };
}

#endif //RAWVIEWER_H
