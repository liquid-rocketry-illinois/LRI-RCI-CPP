#ifndef LRI_CONTROL_PANEL_SPLASH_H
#define LRI_CONTROL_PANEL_SPLASH_H

#include <Windows.h>
#include "glfw/glfw3.h"

#include <chrono>

namespace LRI::RCI {
    class SplashWindow {
        static constexpr std::chrono::milliseconds TIME_OPEN{5000};
        const std::chrono::system_clock::time_point startTime;
        GLFWwindow* window;
        GLuint birdTex;

    public:
        SplashWindow();
        ~SplashWindow();

        SplashWindow(const SplashWindow&) = delete;
        SplashWindow& operator=(const SplashWindow&) = delete;

        void loop();
    };
}

#endif // LRI_CONTROL_PANEL_SPLASH_H
