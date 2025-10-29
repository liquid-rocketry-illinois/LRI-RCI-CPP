#include "hardware/Prompt.h"

namespace LRI::RCI::Prompt {
    // Is there currently an active prompt
    static bool activePrompt;

    // Currently active prompt string
    static std::string prompt;

    // Holding values for prompt responses
    static RCP_GONOGO gng;
    static float val;

    // The currently active prompt data type
    static RCP_PromptDataType type;

    bool isActivePrompt() { return activePrompt; }

    const std::string& get_prompt() { return prompt; }

    RCP_PromptDataType getType() { return type; }

    RCP_GONOGO* getGNGPointer() { return &gng; }

    float* getValPointer() { return &val; }

    void reset() {
        activePrompt = false;
        prompt = "";
    }

    void receiveRCPUpdate(const RCP_PromptInputRequest& req) {
        type = req.type;
        if(type != RCP_PromptDataType_RESET) prompt = req.prompt;
        activePrompt = type != RCP_PromptDataType_RESET;
    }

    bool submitPrompt() {
        bool complete;
        if(type == RCP_PromptDataType_GONOGO) complete = !RCP_promptRespondGONOGO(gng);
        else complete = !RCP_promptRespondFloat(val);
        if(complete) activePrompt = false;
        return complete;
    }
} // namespace LRI::RCI::Prompt
