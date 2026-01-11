#include "hardware/Prompt.h"

#include "hardware/EventLog.h"

namespace LRI::RCI::Prompt {
    // Is there currently an active prompt
    static bool activePrompt;

    // Currently active prompt string
    static std::string prompt;

    // The currently active prompt data type
    static RCP_PromptDataType type;

    bool isActivePrompt() { return getType() != RCP_PromptDataType_RESET; }

    const std::string& get_prompt() { return prompt; }

    RCP_PromptDataType getType() { return RCP_getActivePromptType(); }

    int submitPrompt(bool val) {
        EventLog::getGlobalLog().addPromptResponse(val);
        return RCP_promptRespondGONOGO(val ? RCP_GONOGO_GO : RCP_GONOGO_NOGO);
    }

    int submitPrompt(float val) {
        EventLog::getGlobalLog().addPromptResponse(val);
        return RCP_promptRespondFloat(val);
    }

    RCP_Error receiveRCPUpdate(RCP_PromptInputRequest req) {
        type = req.type;
        if(type != RCP_PromptDataType_RESET) prompt = req.prompt;
        activePrompt = type != RCP_PromptDataType_RESET;
        EventLog::getGlobalLog().addPromptRequest(req);
        return RCP_ERR_SUCCESS;
    }
} // namespace LRI::RCI::Prompt
