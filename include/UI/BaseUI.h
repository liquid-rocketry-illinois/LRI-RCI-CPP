#ifndef BASEUI_H
#define BASEUI_H

#include <set>
#include <utils.h>

namespace LRI::RCI {

    // Abstract interface representing all UI windows
    class BaseUI {
        // A list that contains all the windows to be rendered on a call to renderWindows()
        static std::set<BaseUI*> windows;
        static std::set<BaseUI*> activeWindows;

    protected:
        static StopWatch buttonTimer;

        // Size for all status squares

        explicit BaseUI();
        virtual ~BaseUI();

    public:
        // Timer available to all windows to prevent button spam
        static constexpr long long BUTTON_DELAY = 1; // Seconds

        static constexpr ImVec2 STATUS_SQUARE_SIZE = ImVec2(15, 15);

        // Common colors
        static constexpr ImU32 ENABLED_COLOR = ImU32(0xFF00FF00); // Colors are stored as ABGR
        static constexpr ImU32 STALE_COLOR = ImU32(0xF000CDDB);
        static constexpr ImU32 DISABLED_COLOR = ImU32(0xFF0000FF);
        static constexpr ImU32 REBECCA_PURPLE = ImU32(0xFF993366);
        // Called by the main loop to render and update all windows
        static void renderWindows();

        // Hide all windows that can be hidden (for example TargetChooser cannot be hidden)
        static void hideAll();

        // Function for children to override. This is called to both render and update
        virtual void render() = 0;

        // Show and hide can be overridden for special windows
        virtual void hideWindow();
        virtual void showWindow();

        // Can be overridden if special reset functionality is needed
        virtual void reset();
    };


}

#endif