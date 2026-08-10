#ifndef LRI_CONTROL_PANEL_WINDOW_H
#define LRI_CONTROL_PANEL_WINDOW_H

#include <string>

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
        virtual std::string contextString() = 0;
    };
} // namespace LRI::RCI

#endif // LRI_CONTROL_PANEL_WINDOW_H
