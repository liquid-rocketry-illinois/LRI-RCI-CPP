#ifndef PROMPTVIEWER_H
#define PROMPTVIEWER_H

#include "WModule.h"

namespace LRI::RCI {
    class PromptViewer : public WModule {
        static int CLASSID;

        const int classid;

    public:
        explicit PromptViewer();
        ~PromptViewer() override = default;

        void render() override;
    };
}

#endif //PROMPTVIEWER_H
