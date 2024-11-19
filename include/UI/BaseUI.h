#ifndef BASEUI_H
#define BASEUI_H

#include <set>

namespace LRI::RCI {
    class BaseUI {
        static std::set<BaseUI*> windows;

    protected:
        explicit BaseUI() = default;

    public:
        static void renderWindows();
        static void hideAll();

        virtual ~BaseUI() = default;
        virtual void render() = 0;
        virtual void hideWindow();
        virtual void showWindow();
    };


}

#endif