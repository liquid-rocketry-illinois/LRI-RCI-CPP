#ifndef BASEUI_H
#define BASEUI_H

#include <set>
#include <utils.h>

namespace LRI::RCI {
    class BaseUI {
        static std::set<BaseUI*> windows;

    protected:
        static StopWatch buttonTimer;
        static constexpr long long buttonDelay = 1;

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