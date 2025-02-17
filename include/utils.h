#ifndef UTILS_H
#define UTILS_H

#include <Windows.h>
#include <string>
#include "GLFW/glfw3.h"
#include "imgui.h"
#include "RCP_Host/RCP_Host.h"

// A mish-mash of various helper functions and stuff

namespace LRI::RCI {
    // Background purple color. CSS Rebecca Purple
    static constexpr ImVec4 BACKGROUND_COLOR{0.4f, 0.2f, 0.6f, 1.0f};

    // Pointers to the regular and bold fonts
    extern ImFont* font_regular;
    extern ImFont* font_bold;

    // Scaling factor for hi-dpi screens
    extern float scaling_factor;


    // Helpers for scaling floats and ImVec2's by scaling_factor
    ImVec2 scale(const ImVec2& vec);
    float scale(float val);

    // Helpers to organize the various ImGui related calls
    void imgui_init(GLFWwindow* window);
    void imgui_prerender(GLFWwindow* window);
    void imgui_postrender(GLFWwindow* window);
    void imgui_shutdown(GLFWwindow* window);

    // Stopwatch class for time tracking
    class StopWatch final {
        // The last time the stopwatch was reset
        time_t lastClock{};

    public:
        StopWatch();
        ~StopWatch() = default;

        // Resets time to zero
        void reset();

        // Gets time since last reset
        [[nodiscard]] time_t timeSince() const;
    };


    // Small helper
    std::string devclassToString(RCP_DeviceClass_t devclass);

    // ImVec2 operators
    ImVec2 operator+(ImVec2 const& v1, ImVec2 const& v2);
    ImVec2 operator-(ImVec2 const& v1, ImVec2 const& v2);
    ImVec2 operator*(ImVec2 const& v1, ImVec2 const& v2);
    ImVec2 operator*(ImVec2 const& v1, float constant);
    ImVec2 operator/(ImVec2 const& v1, ImVec2 const& v2);
    ImVec2 operator/(ImVec2 const& v1, float constant);

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

}

#include "RingBuffer.inl"

#endif //UTILS_H
