#ifndef WMODULE_H
#define WMODULE_H

#include "imgui.h"
#include "utils.h"

namespace LRI::RCI {
    // The abstract class for an individual module. These are what contain the actual rendering code.
    // One thing to note is that each module has its own static variable CLASSID, which is then
    // incremented and assigned to each instantiated class in the const int classid. This is used to track
    // individual instances of modules so that that can uniquely identify themselves to imgui.
    class WModule {
        static int CLASSID;

    protected:
        static StopWatch SPAM_TIMER;

        const int classid;

    public:
        // Action delays, in seconds
        static constexpr float SPAM_DELAY = 1;
        static constexpr float CONFIRM_HOLD_TIME = 3;

        WModule();
        virtual ~WModule() = default;
        virtual void render() = 0;
    };
} // namespace LRI::RCI

#endif // WMODULE_H
