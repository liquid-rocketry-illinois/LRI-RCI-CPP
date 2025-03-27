#ifndef PROMPTVIEWER_H
#define PROMPTVIEWER_H

#include <string>

#include "UI/BaseUI.h"
#include "RCP_Host/RCP_Host.h"

namespace LRI::RCI {
    class PromptViewer : public BaseUI {
        static int CLASSID;

        const int classid;
        const bool standaloneWindow;
        const ImVec2 startPos;
        const ImVec2 startSize;

    public:
        explicit PromptViewer(bool standaloneWindow, const ImVec2&& startPos = {0, 0},
                              const ImVec2&& startSize = {0, 0});
        ~PromptViewer() override = default;

        void render() override;
    };
}

#endif //PROMPTVIEWER_H
