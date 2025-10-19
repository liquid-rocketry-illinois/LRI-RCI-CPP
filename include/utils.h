#ifndef UTILS_H
#define UTILS_H

#include <chrono>
#include <string>

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

    // Scaling factor for hi-dpi screens. Not perfect but its good enough
    extern float scaling_factor;

    // ImVec2 operators
    inline ImVec2 operator+(ImVec2 const& v1, ImVec2 const& v2) { return {v1.x + v2.x, v1.y + v2.y}; }
    inline ImVec2 operator-(ImVec2 const& v1, ImVec2 const& v2) { return {v1.x - v2.x, v1.y - v2.y}; }
    inline ImVec2 operator*(ImVec2 const& v1, ImVec2 const& v2) { return {v1.x * v2.x, v1.y * v2.y}; }
    inline ImVec2 operator*(ImVec2 const& v1, float constant) { return {v1.x * constant, v1.y * constant}; }
    inline ImVec2 operator/(ImVec2 const& v1, ImVec2 const& v2) { return {v1.x / v2.x, v1.y / v2.y}; }
    inline ImVec2 operator/(ImVec2 const& v1, float constant) { return {v1.x / constant, v1.y / constant}; }

    // Helpers for scaling floats and ImVec2's by scaling_factor
    inline ImVec2 scale(const ImVec2& vec) { return vec * scaling_factor; }
    inline float scale(float val) { return val * scaling_factor; }

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

    // Small helper
    std::string devclassToString(RCP_DeviceClass devclass);


    // Class definition for RingBuffer. See RingBuffer.inl
    template<typename T, T ret = 0>
    class RingBuffer {
        uint32_t buffersize;
        uint32_t datastart;
        uint32_t dataend;
        T* data;

    public:
        explicit RingBuffer(uint32_t buffersize);
        RingBuffer(RingBuffer& other);
        ~RingBuffer();

        [[nodiscard]] uint32_t size() const;
        [[nodiscard]] bool isEmpty() const;
        [[nodiscard]] bool isFull() const;
        [[nodiscard]] uint32_t capacity() const;
        T pop();
        T peek() const;
        void push(T value);
        void clear();
    };

    class ThreadStopException : public std::runtime_error {
    public:
        explicit ThreadStopException(const std::string& what) : runtime_error(what) {}
        ~ThreadStopException() override = default;
    };

    class RCPStreamException : public std::runtime_error {
    public:
        explicit RCPStreamException(const std::string& what) : runtime_error(what) {}
        ~RCPStreamException() override = default;
    };

    // Loading a window layout must happen between imgui frames, hence why this structure is used to communicate
    // outside the WModule/Windowlet abstraction tree
    class IniFilePath {
        friend class TargetChooser;
        std::string path;

    public:
        std::string getPath() {
            std::string copy = path;
            path = "";
            return copy;
        }

        [[nodiscard]] bool empty() const { return path.empty(); }
    };

    extern IniFilePath iniFilePath;
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

#include "RingBuffer.inl"

#endif // UTILS_H
