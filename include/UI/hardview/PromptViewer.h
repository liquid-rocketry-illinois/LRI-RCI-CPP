#ifndef PROMPTVIEWER_H
#define PROMPTVIEWER_H

#include "../WModule.h"

// Window module for showing prompts
namespace LRI::RCI {
    class PromptViewer : public WModule {
    public:
        explicit PromptViewer() = default;
        ~PromptViewer() override = default;

        void render() override;
    };
} // namespace LRI::RCI

#endif // PROMPTVIEWER_H
