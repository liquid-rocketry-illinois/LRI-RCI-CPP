#ifndef PROMPT_H
#define PROMPT_H

#include <string>

#include "RCP_Host/RCP_Host.h"

// Singleton class for representing prompt state
namespace LRI::RCI {
    class Prompt {
        Prompt() = default;
        ~Prompt() = default;

        // Is there currently an active prompt
        bool activePrompt;

        // Currently active prompt string
        std::string prompt;

        // Holding values for prompt responses
        RCP_GONOGO gng;
        float val;

        // The currently active prompt data type
        RCP_PromptDataType type;

    public:
        // Get singleton instance
        static Prompt* getInstance();

        // Gettters for class members
        [[nodiscard]] bool isActivePrompt() const;
        [[nodiscard]] const std::string& get_prompt() const;
        [[nodiscard]] RCP_PromptDataType getType() const;

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
    };
} // namespace LRI::RCI

#endif // PROMPT_H
