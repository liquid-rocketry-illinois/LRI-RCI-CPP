#ifndef PROMPTVIEWER_H
#define PROMPTVIEWER_H

#include <string>

#include "UI/BaseUI.h"
#include "RCP_Host/RCP_Host.h"

namespace LRI::RCI {
    class PromptViewer : public BaseUI {
        static PromptViewer* instance;

        PromptViewer() = default;

        std::string prompt;
        RCP_PromptDataType_t type = RCP_PromptDataType_GONOGO;
        bool gonogo = false;
        float value = 0;

    public:
        static PromptViewer* getInstance();

        void render() override;
        void setPrompt(const RCP_PromptInputRequest& req);
        ~PromptViewer() override = default;
    };
}

#endif //PROMPTVIEWER_H
