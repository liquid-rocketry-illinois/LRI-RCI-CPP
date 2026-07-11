#ifndef LRI_CONTROL_PANEL_WINDOW_H
#define LRI_CONTROL_PANEL_WINDOW_H

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
        virtual bool shouldClose() = 0;
    };

    using CaptionFunction = std::function<bool(LONG)>;
    struct WindowInfo {
        CaptionFunction cfunc;
        WNDPROC oldProc;
    };

    LRESULT borderlessCallback(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    GLFWwindow* setupBorderlessWindow(WindowInfo* usrptr);
} // namespace LRI::RCI

#endif // LRI_CONTROL_PANEL_WINDOW_H
