#ifndef WINDOWLET_H
#define WINDOWLET_H

#include <set>
#include <string>

#include "WModule.h"

namespace LRI::RCI {
    class Windowlet {
        static std::set<Windowlet*> windows;

    protected:
        const std::string title;
        const std::set<WModule*> modules;
        explicit Windowlet(const std::string& title, const std::set<WModule*>& modules);
        virtual ~Windowlet();

    public:
        static void renderWindowlets();

        virtual void render();
    };

    class ControlWindowlet final : public Windowlet {
        ControlWindowlet();

    public:
        static ControlWindowlet* getInstance();

        void render() override;
    };
}

#endif //WINDOWLET_H
