#ifndef LRI_CONTROL_PANEL_WINDOW_H
#define LRI_CONTROL_PANEL_WINDOW_H

#include <functional>
#include <set>
#include <vector>

#include <Windows.h>

#include "glfw/glfw3.h"

#include "UI/Windowlet.h"
#include "UI/TargetChooser.h"

namespace LRI::RCI {
    class TargetChooser;

    class Window {
        static constexpr float CAPTION_SIZE = 40;
        static LRESULT borderlessProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

        GLFWwindow* window;
        WNDPROC oldProc;

        std::set<Windowlet*> windowlets;
        std::vector<std::function<void()>> preframes;

        TargetChooser chooser;

        void renderTitlebar();
        void renderBackground();

    public:
        Window();
        ~Window();
        void show();
        void preframe(std::function<void()> func);

        void registerWindowlet(Windowlet* w);
        void unregisterWindowlets();
    };
}

#endif // LRI_CONTROL_PANEL_WINDOW_H
