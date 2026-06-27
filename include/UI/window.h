#ifndef LRI_CONTROL_PANEL_WINDOW_H
#define LRI_CONTROL_PANEL_WINDOW_H

#include <set>
#include <functional>
#include "Windows.h"
#include "glfw/glfw3.h"

namespace LRI::RCI {
    namespace splash {
        void show();
    }

    void show();

    class Windowlet {
    public:
        virtual ~Windowlet() = default;
        virtual void render() = 0;
    };

    using CaptionFunction = std::function<bool(LONG)>;
    struct WindowInfo {
        CaptionFunction cfunc;
        WNDPROC oldProc;
    };

    LRESULT borderlessCallback(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    GLFWwindow* setupBorderlessWindow(WindowInfo* usrptr);
    // class Window {
    //     GLFWwindow* window;
    //     HWND hwnd;
    //     WNDPROC oldProc;
    //
    //     std::set<Windowlet*> windowlets;
    //
    //     void frame();
    //     void renderCaption();
    //     void renderTargetPopup();
    //     bool inCaption(LONG cursory) const;
    //
    // public:
    //     Window();
    //     ~Window();
    //
    //     Window(Window&) = delete;
    //     Window& operator=(const Window&) = delete;
    //
    //     void loop();
    //     void registerWindowlet(Windowlet* w);
    // };
} // namespace LRI::RCI

#endif // LRI_CONTROL_PANEL_WINDOW_H
