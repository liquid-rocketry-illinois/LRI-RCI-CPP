#ifndef WINDOWLET_H
#define WINDOWLET_H

#include <set>
#include <string>

#include "WModule.h"
#include "TargetChooser.h"

namespace LRI::RCI {
    class Windowlet {
    protected:
        static std::set<Windowlet*> windows;

        const std::string title;
        const std::set<WModule*> modules;
        explicit Windowlet(std::string  title, const std::set<WModule*>& modules);
        virtual ~Windowlet();

    public:
        static void renderWindowlets();

        virtual void render();
    };

    class ControlWindowlet final : public Windowlet {
        friend class TargetChooser;

        ControlWindowlet();
        void cleanup();

    public:
        ~ControlWindowlet() override = delete;
        static ControlWindowlet* getInstance();

        void render() override;
    };
}

#endif //WINDOWLET_H
