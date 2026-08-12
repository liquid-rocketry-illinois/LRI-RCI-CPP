#ifndef UTILS_H
#define UTILS_H

#include <chrono>
#include <string>
#include <filesystem>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

#include "GLFW/glfw3.h"
#include "RCP_Host/RCP_Host.h"
#include "imgui.h"

// A mish-mash of various helper functions and stuff

namespace LRI::RCI {
    // Background purple color. CSS Rebecca Purple
    static constexpr ImVec4 BACKGROUND_COLOR{0.f, 0.f, 0.f, 0.f};

    // Pointers to the regular, bold, and italic fonts
    extern ImFont* font_regular;
    extern ImFont* font_bold;
    extern ImFont* font_italic;

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

        [[nodiscard]] std::chrono::milliseconds millis() const {
            return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - lastClock);
        }
    };

    // The definition for this function is in rendering.cpp, since this function deals with some static members
    void setIniFileForNextFrame(const std::string& path);

    // Small helper
    std::string devclassToString(RCP_DeviceClass devclass);

    [[nodiscard]] const std::filesystem::path& getRoamingFolder();
    void detectRoamingFolder();
    void preventScreenTurnoff();
    void allowScreenTurnoff();
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
