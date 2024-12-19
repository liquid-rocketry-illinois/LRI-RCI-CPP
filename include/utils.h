#ifndef UTILS_H
#define UTILS_H

#include <Windows.h>
#include "GLFW/glfw3.h"
#include "imgui.h"

namespace LRI::RCI {
    static constexpr ImVec4 BACKGROUND_COLOR {0.4f, 0.2f, 0.6f, 1.0f};

    extern ImFont* font_regular;
    extern ImFont* font_bold;
    extern float scaling_factor;
    extern HANDLE com;

    ImVec2 scale(const ImVec2& vec);
    float scale(float val);
    void imgui_init(GLFWwindow* window);
    void imgui_prerender(GLFWwindow* window);
    void imgui_postrender(GLFWwindow* window);
    void imgui_shutdown(GLFWwindow* window);

    void renderCOMChooser();
    void renderEStop();

    class StopWatch final {
        time_t lastClock{};

    public:
        StopWatch();
        ~StopWatch() = default;

        void reset();
        [[nodiscard]] time_t timeSince() const;
    };

    // ImVec2 operators
    ImVec2 operator+(ImVec2 const& v1, ImVec2 const& v2);
    ImVec2 operator-(ImVec2 const& v1, ImVec2 const& v2);
    ImVec2 operator*(ImVec2 const& v1, ImVec2 const& v2);
    ImVec2 operator*(ImVec2 const& v1, float constant);
    ImVec2 operator/(ImVec2 const& v1, ImVec2 const& v2);
    ImVec2 operator/(ImVec2 const& v1, float constant);

    template <typename T, T ret = 0>
    class RingBuffer {
        uint32_t buffersize;
        uint32_t datastart;
        uint32_t dataend;
        T* data;

    public:
        explicit RingBuffer(uint32_t buffersize);
        ~RingBuffer();

        [[nodiscard]] uint32_t length() const;
        [[nodiscard]] uint32_t size() const;
        T pop();
        T peek() const;
        void push(T value);
    };

}

#include "RingBuffer.inl"
#endif //UTILS_H
