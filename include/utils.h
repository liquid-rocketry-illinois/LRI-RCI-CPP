#ifndef UTILS_H
#define UTILS_H

#include <chrono>
#include <filesystem>
#include <string>

#include "RCP_Host/RCP_Host.h"
#include "imgui.h"

// A mish-mash of various helper functions and stuff

namespace LRI::RCI {
    // Stopwatch class for time tracking
    class StopWatch final {
        // The last time the stopwatch was reset
        std::chrono::time_point<std::chrono::system_clock> lastClock;

    public:
        StopWatch() : lastClock(std::chrono::system_clock::now()) {}
        ~StopWatch() = default;

        // Resets time to zero
        void reset() { lastClock = std::chrono::system_clock::now(); }

        // Gets time since last reset
        [[nodiscard]] float timeSince() const {
            const std::chrono::duration<float> elapsed = std::chrono::system_clock::now() - lastClock;
            return elapsed.count();
        }
    };

    // The definition for this function is in rendering.cpp, since this function deals with some static members
    void setIniFileForNextFrame(const std::string& path);

    // Small helper
    std::string devclassToString(RCP_DeviceClass devclass);

    [[nodiscard]] const std::filesystem::path& getRoamingFolder();
    void detectRoamingFolder();
} // namespace LRI::RCI

namespace ImGui {
    // A more "imgui"ish way of doing the class below
    bool TimedButton(const char* label, LRI::RCI::StopWatch& sw, const ImVec2& size = ImVec2(0, 0));

    // A button that tracks how long it has been held down for
    class TimedButton {
        const char* label;

        LRI::RCI::StopWatch timer;
        bool clicked;

    public:
        // Constructor just takes the button label
        explicit TimedButton(const char* label);

        // Render the button, and return if the button is held
        bool render();

        // Return the duration the button has been held for
        [[nodiscard]] float getHoldTime() const;
    };
} // namespace ImGui

#endif // UTILS_H
