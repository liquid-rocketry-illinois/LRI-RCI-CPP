#ifndef LRI_CONTROL_PANEL_SPLASH_H
#define LRI_CONTROL_PANEL_SPLASH_H

#include <Windows.h>
#include "glfw/glfw3.h"
#include "utils.h"

namespace LRI::RCI {
    class Splash {
        GLFWwindow* window;
        StopWatch timer;

    public:
        Splash();
        ~Splash();
        void show();
    };
}

#endif // LRI_CONTROL_PANEL_SPLASH_H
