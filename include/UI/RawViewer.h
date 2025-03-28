#ifndef RAWVIEWER_H
#define RAWVIEWER_H

#include "WModule.h"

namespace LRI::RCI {
    // Window which shows the output to the custom device class from RCP
    class RawViewer : public WModule {
        static int CLASSID;

        const int classid;

    public:
        explicit RawViewer();
        ~RawViewer() override = default;

        // Overridden render function
        void render() override;
    };
}

#endif //RAWVIEWER_H
