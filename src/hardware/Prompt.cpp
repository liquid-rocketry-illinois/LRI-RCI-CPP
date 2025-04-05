#include "hardware/Prompt.h"

namespace LRI::RCI {
    Prompt* Prompt::getInstance() {
        static Prompt* instance = nullptr;
        if(instance == nullptr) instance = new Prompt();
        return instance;
    }

    bool Prompt::is_active_prompt() const {
        return activePrompt;
    }

    const std::string& Prompt::get_prompt() const {
        return prompt;
    }

    RCP_PromptDataType Prompt::getType() const {
        return type;
    }

    RCP_GONOGO* Prompt::getGNGPointer() {
        return &gng;
    }

    float* Prompt::getValPointer() {
        return &val;
    }

    void Prompt::reset() {
        activePrompt = false;
        prompt = "";
    }

    void Prompt::receiveRCPUpdate(const RCP_PromptInputRequest& req) {
        type = req.type;
        prompt = req.prompt;
        activePrompt = true;
    }

    bool Prompt::submitPrompt() {
        bool complete;
        if(type == RCP_PromptDataType_GONOGO) complete = !RCP_promptRespondGONOGO(gng);
        else complete = !RCP_promptRespondFloat(val);
        if(complete) activePrompt = false;
        return complete;
    }
}
