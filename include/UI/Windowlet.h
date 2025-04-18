#ifndef WINDOWLET_H
#define WINDOWLET_H

#include <set>
#include <string>

#include "interfaces/RCP_Interface.h"
#include "WModule.h"
#include "TargetChooser.h" // Linters will complain this is unused but it actually is needed. Header stuff

namespace LRI::RCI {
    /*
     * Windowlets are the basis of the whole program. These are the individual little windows that appear
     * inside the main window when you launch the program (hence the name "windowlets"). Each windowlet is
     * what contains the different module types, and renders them. In the main function, the only thing needed
     * to render the whole program is to call Windowlet::renderWindowlets, which iterates through each windowlet
     * calling its render function, which in turn iterates through each of its modules and calls its render
     * function.
     */

    class Windowlet {
    protected:
        // The set of all windowlets, besides the control windowlet
        static std::set<Windowlet*> windows;

        // An individual windowlets title, and the vector of its modules. This has to be a vector, since
        // the order of the modules matters. If you use a set, the modules will get ordered by the value
        // of their pointer, which means the exact order of modules is indeterminate.
        const std::string title;
        const std::vector<WModule*> modules;

        // This constructor is protected since a regular user shouldn't be able to control the value of addToSet,
        // this parameter is present purely for the control window
        explicit Windowlet(std::string title, const std::vector<WModule*>& modules, bool addToSet);

    public:
        // Windowlet constructor. Takes the windowlet title and its modules
        explicit Windowlet(std::string title, const std::vector<WModule*>& modules);
        virtual ~Windowlet();

        // The global windowlets renderer
        static void renderWindowlets();

        // The individual windowlets renderer
        virtual void render();
    };

    /*
     * The control windowlet is the windowlet which actually contains the TargetChooser module. This windowlet
     * is special because it always needs to be visible and active, it cant be merged with other windowlets, and
     * it controls the entire rest of the program.
     */

    class ControlWindowlet final : public Windowlet {
        friend class TargetChooser;

        ControlWindowlet();
        ~ControlWindowlet() override = default;

        RCP_Interface* interf;
        std::string inipath;

    public:
        static ControlWindowlet* getInstance();

        void render() override;

        // As opposed to a normal windowlet, the control windowlet needs a few extra special functions.
        // First of all, it needs to be able to clean up the rest of the program, and close all the windowlets
        // that were open due to the active configuration. It also needs to be able to provide a pointer to the
        // interface RCP should use to communicate with the target.
        void cleanup();
        RCP_Interface* getInterf() const;
    };
}

#endif //WINDOWLET_H
