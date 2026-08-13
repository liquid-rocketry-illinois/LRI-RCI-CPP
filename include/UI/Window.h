#ifndef LRI_CONTROL_PANEL_WINDOW_H
#define LRI_CONTROL_PANEL_WINDOW_H

#include <functional>
#include <set>
#include <vector>

#include <Windows.h>

#include "glfw/glfw3.h"

#include "UI/Windowlet.h"

namespace LRI::RCI {
    class Window {
        static constexpr float CAPTION_SIZE = 40;
        static LRESULT borderlessProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

        GLFWwindow* window;
        WNDPROC oldProc;

        std::set<Windowlet*> windowlets;
        std::vector<std::function<void()>> preframes;

        int classid;

    public:
        Window();
        ~Window();
        void show();
        void preframe(std::function<void()> func);

        void registerWindowlet(Windowlet* w);
        void unregisterWindowlet(Windowlet* w);

        int getClassid();
    };
}

#endif // LRI_CONTROL_PANEL_WINDOW_H
