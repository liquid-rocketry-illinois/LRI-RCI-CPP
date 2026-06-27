#ifndef LRI_CONTROL_PANEL_WINDOW_H
#define LRI_CONTROL_PANEL_WINDOW_H

#include "Windows.h"
#include "glfw/glfw3.h"
#include <set>

namespace LRI::RCI {
    class Windowlet {
    public:
        virtual ~Windowlet() = default;
        virtual void render() = 0;
    };

    class Window {
        GLFWwindow* window;
        HWND hwnd;
        WNDPROC oldProc;

        std::set<Windowlet*> windowlets;

        void frame();
        void renderCaption();
        void renderTargetPopup();
        bool inCaption(LONG cursory) const;

    public:
        friend LRESULT borderlessCallback(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
        Window();
        ~Window();

        Window(Window&) = delete;
        Window& operator=(const Window&) = delete;

        void loop();
        void registerWindowlet(Windowlet* w);
    };
}

#endif // LRI_CONTROL_PANEL_WINDOW_H
