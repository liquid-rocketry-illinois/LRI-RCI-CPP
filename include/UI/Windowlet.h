#ifndef WINDOWLET_H
#define WINDOWLET_H

#include <set>
#include <string>

#include "interfaces/RCP_Interface.h"
#include "WModule.h"
#include "TargetChooser.h"

namespace LRI::RCI {
    class Windowlet {
    protected:
        static std::set<Windowlet*> windows;

        const std::string title;
        const std::set<WModule*> modules;

    public:
        explicit Windowlet(std::string  title, const std::set<WModule*>& modules);
        virtual ~Windowlet();
        static void renderWindowlets();

        virtual void render();
    };

    class ControlWindowlet final : public Windowlet {
        friend class TargetChooser;

        ControlWindowlet();
        ~ControlWindowlet() override = default;

        void cleanup();
        RCP_Interface* interf;

    public:
        static ControlWindowlet* getInstance();

        void render() override;
        RCP_Interface* getInterf() const;
    };
}

#endif //WINDOWLET_H
