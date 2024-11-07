#ifndef BASEUI_H
#define BASEUI_H

namespace LRI::RCI {
    class BaseUI {
    protected:
        explicit BaseUI();

    public:
        virtual ~BaseUI() = default;
        virtual void render() = 0;
        virtual void destroy();
    };

    void renderWindows();
}

#endif