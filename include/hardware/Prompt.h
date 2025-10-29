#ifndef PROMPT_H
#define PROMPT_H

#include <string>

#include "RCP_Host/RCP_Host.h"

// Singleton class for representing prompt state
namespace LRI::RCI::Prompt {
    // Gettters for class members
    [[nodiscard]] bool isActivePrompt();
    [[nodiscard]] const std::string& get_prompt();
    [[nodiscard]] RCP_PromptDataType getType();

    // Get pointers to the holding variables, that viewer classes can
    // modify to respond to the prompt
    [[nodiscard]] RCP_GONOGO* getGNGPointer();
    [[nodiscard]] float* getValPointer();

    // Reset singleton state
    void reset();

    // Submit the latest prompt data
    bool submitPrompt();

    // Receive prompt requests from RCP
    void receiveRCPUpdate(const RCP_PromptInputRequest& req);
} // namespace LRI::RCI

#endif // PROMPT_H
