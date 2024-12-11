#ifndef BASEUI_H
#define BASEUI_H

#include <set>
#include <utils.h>

namespace LRI::RCI {
    class BaseUI {
        static std::set<BaseUI*> windows;

    protected:
        static StopWatch buttonTimer;
        static constexpr long long buttonDelay = 1;

        static constexpr ImU32 ENABLED_COLOR = ImU32(0xFF00FF00); // Colors are stored as ABGR
        static constexpr ImU32 STALE_COLOR = ImU32(0xF000CDDB);
        static constexpr ImU32 DISABLED_COLOR = ImU32(0xFF0000FF);
        static constexpr ImU32 REBECCA_PURPLE = ImU32(0xFF993366);

        explicit BaseUI() = default;

    public:
        static void renderWindows();
        static void hideAll();

        virtual ~BaseUI() = default;
        virtual void render() = 0;
        virtual void hideWindow();
        virtual void showWindow();
    };


}

#endif