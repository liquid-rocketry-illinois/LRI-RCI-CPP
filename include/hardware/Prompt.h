#ifndef PROMPT_H
#define PROMPT_H

#include <string>

#include "RCP_Host/RCP_Host.h"

// Singleton class for representing prompt state
namespace LRI::RCI::Prompt {
    // Getters for class members
    [[nodiscard]] bool isActivePrompt();
    [[nodiscard]] const std::string& get_prompt();
    [[nodiscard]] RCP_PromptDataType getType();

    // Submit the latest prompt data
    int submitPrompt(float val);
    int submitPrompt(bool val);

    // Receive prompt requests from RCP
    RCP_Error receiveRCPUpdate(RCP_PromptInputRequest req);
} // namespace LRI::RCI

#endif // PROMPT_H
