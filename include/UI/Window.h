#ifndef LRI_CONTROL_PANEL_WINDOW_H
#define LRI_CONTROL_PANEL_WINDOW_H

#include <filesystem>
#include <functional>
#include <set>
#include <vector>

#include <Windows.h>

#include "glfw/glfw3.h"

#include "UI/Windowlet.h"
#include "UI/TargetChooser.h"

#include "hardware/json.h"

#include "interfaces/RCP_Interface.h"

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

        std::string openTarget;
        std::string openInterf;

        void renderTitlebar();
        void renderBackground();

    public:
        Window();
        ~Window();
        void show();
        void preframe(std::function<void()> func);

        void registerWindowlet(Windowlet* w);
        void unregisterWindowlets();
        void startTarget(RCP_Interface* interf, const TargetConfig& configPath);
    };
}

#endif // LRI_CONTROL_PANEL_WINDOW_H
