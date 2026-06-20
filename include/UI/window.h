#ifndef LRI_CONTROL_PANEL_WINDOW_H
#define LRI_CONTROL_PANEL_WINDOW_H

#include "Windows.h"
#include "glfw/glfw3.h"

namespace LRI::RCI {

    class Window {
        GLFWwindow* window;
        HWND hwnd;
        WNDPROC oldProc;

        void frame();
        bool inCaption(LONG cursorY);

    public:
        friend LRESULT borderlessCallback(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
        Window();
        ~Window();

        Window(Window&) = delete;
        Window& operator=(const Window&) = delete;

        void loop();
    };
}

#endif // LRI_CONTROL_PANEL_WINDOW_H
