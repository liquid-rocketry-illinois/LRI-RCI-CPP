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
    protected:
        static StopWatch buttonTimer;

    public:
        // Delay after actions in the UI to prevent spam
        static constexpr long long BUTTON_DELAY = 1; // Seconds

        static constexpr ImVec2 STATUS_SQUARE_SIZE = {15, 15};

        // Common colors
        static constexpr ImU32 ENABLED_COLOR = 0xFF00FF00; // Colors are stored as ABGR
        static constexpr ImU32 STALE_COLOR = 0xF000CDDB;
        static constexpr ImU32 DISABLED_COLOR = 0xFF0000FF;
        static constexpr ImU32 REBECCA_PURPLE = 0xFF993366;

        WModule() = default;
        virtual ~WModule() = default;
        virtual void render() = 0;
    };
}

#endif //WMODULE_H
