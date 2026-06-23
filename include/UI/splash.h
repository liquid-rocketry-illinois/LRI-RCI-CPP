#ifndef LRI_CONTROL_PANEL_SPLASH_H
#define LRI_CONTROL_PANEL_SPLASH_H

#include <Windows.h>
#include "glfw/glfw3.h"

#include <chrono>

namespace LRI::RCI {
    class SplashWindow {
        static constexpr std::chrono::milliseconds TIME_OPEN{
#ifdef RCIDEBUG
            1000
#else
            5000
#endif
        };

        const std::chrono::system_clock::time_point startTime;
        GLFWwindow* window;

    public:
        SplashWindow();
        ~SplashWindow();

        SplashWindow(const SplashWindow&) = delete;
        SplashWindow& operator=(const SplashWindow&) = delete;

        void loop();
    };
} // namespace LRI::RCI

#endif // LRI_CONTROL_PANEL_SPLASH_H
