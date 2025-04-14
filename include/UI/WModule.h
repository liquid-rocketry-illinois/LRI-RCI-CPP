#ifndef WMODULE_H
#define WMODULE_H

#include "imgui.h"
#include "utils.h"

namespace LRI::RCI {
    class WModule {
    protected:
        static StopWatch buttonTimer;

    public:
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
