#ifndef PROMPT_H
#define PROMPT_H

#include <string>

#include "RCP_Host/RCP_Host.h"

namespace LRI::RCI {
    class Prompt {
        Prompt() = default;
        ~Prompt() = default;

        bool activePrompt;
        std::string prompt;
        RCP_GONOGO gng;
        float val;
        RCP_PromptDataType type;

    public:
        static Prompt* getInstance();

        [[nodiscard]] bool is_active_prompt() const;
        [[nodiscard]] const std::string& get_prompt() const;
        [[nodiscard]] RCP_PromptDataType getType() const;
        [[nodiscard]] RCP_GONOGO* getGNGPointer();
        [[nodiscard]] float* getValPointer();
        void reset();

        bool submitPrompt();

        void receiveRCPUpdate(const RCP_PromptInputRequest& req);
    };
}

#endif //PROMPT_H
