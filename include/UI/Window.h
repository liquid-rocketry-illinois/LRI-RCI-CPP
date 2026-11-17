#ifndef LRI_CONTROL_PANEL_WINDOW_H
#define LRI_CONTROL_PANEL_WINDOW_H

#include <functional>
#include <set>
#include <vector>

#include <Windows.h>

#include "glfw/glfw3.h"

#include "UI/Windowlet.h"

#include "hardware/json.h"

#include "interfaces/RCP_Interface.h"

namespace LRI::RCI {
    class WindowTitlebarFunctions {
    public:
        virtual void extraTitlebarElements(float textY) = 0;
        virtual void menuPopupElements() = 0;
        virtual bool lockCloseButton() { return false; }

        WindowTitlebarFunctions() = default;
        virtual ~WindowTitlebarFunctions() = default;

        WindowTitlebarFunctions(const WindowTitlebarFunctions&) = delete;
        WindowTitlebarFunctions& operator=(const WindowTitlebarFunctions&) = delete;
    };

    class Window {
        static constexpr float CAPTION_SIZE = 40;
        static LRESULT borderlessProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

        GLFWwindow* window;
        WNDPROC oldProc;

        std::set<Windowlet*> windowlets;
        WindowTitlebarFunctions* wtf;
        bool defaultWindowletsActive;
        std::vector<std::function<void()>> preframes;

        std::set<Windowlet*> setupWindowlets;

        bool debugChordPrev;
        bool showDebug;

        void renderTitlebar();
        void renderBackground();
        void renderDebug();

    public:
        Window();
        ~Window();
        void show();
        void preframe(std::function<void()> func);

        void setWindowlets(WindowTitlebarFunctions* newWtf, std::set<Windowlet*>&& newWindowlets);
        void resetWindowlets();

        HWND getHandle() const;
    };
}

#endif // LRI_CONTROL_PANEL_WINDOW_H
