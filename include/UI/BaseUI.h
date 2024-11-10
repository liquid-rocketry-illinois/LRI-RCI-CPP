#ifndef BASEUI_H
#define BASEUI_H

namespace LRI::RCI {
    class BaseUI {
    protected:
        explicit BaseUI();

    public:
        virtual ~BaseUI() = default;
        virtual void render() = 0;
        virtual void hideWindow();
        virtual void showWindow();
    };

    void renderWindows();
}

#endif