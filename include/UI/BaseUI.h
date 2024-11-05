#ifndef BASEUI_H
#define BASEUI_H

namespace LRI::RCI {
    class BaseUI {
    protected:
        explicit BaseUI();

    public:
        virtual void render() = 0;
    };

    void renderWindows();
}

#endif