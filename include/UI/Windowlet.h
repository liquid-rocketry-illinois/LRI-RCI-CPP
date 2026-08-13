#ifndef WINDOWLET_H
#define WINDOWLET_H

#include <string>
#include <vector>

#include "imgui.h"
#include "utils.h"

namespace LRI::RCI {
    // Manual forward declaration needed to avoid circular header dependency
    class Window;

    // The abstract class for an individual module. These are what contain the actual rendering code.
    // One thing to note is that each module has its own static variable CLASSID, which is then
    // incremented and assigned to each instantiated class in the const int classid. This is used to track
    // individual instances of modules so that that can uniquely identify themselves to imgui.
    class WModule {
        static int CLASSID;

    protected:
        static StopWatch buttonTimer;

        const int classid;

    public:
        // Delay after actions in the UI to prevent spam
        static constexpr float BUTTON_DELAY = 0.125; // Seconds
        static constexpr float CONFIRM_HOLD_TIME = 3;

        static constexpr ImVec2 STATUS_SQUARE_SIZE = {15, 15};

        // Common colors
        static constexpr ImU32 ENABLED_COLOR = 0xFF00FF00; // Colors are stored as ABGR
        static constexpr ImU32 STALE_COLOR = 0xF000CDDB;
        static constexpr ImU32 DISABLED_COLOR = 0xFF0000FF;
        static constexpr ImU32 REBECCA_PURPLE = 0xFF993366;
        static constexpr ImU32 WHITE_COLOR = 0xFFFFFFFF;

        explicit WModule();
        virtual ~WModule() = default;
        virtual void render() = 0;
    };

    /*
     * Windowlets are the basis of the whole program. These are the individual little windows that appear
     * inside the main window when you launch the program (hence the name "windowlets"). Each windowlet is
     * what contains the different module types, and renders them. In the main function, the only thing needed
     * to render the whole program is to call Windowlet::renderWindowlets, which iterates through each windowlet
     * calling its render function, which in turn iterates through each of its modules and calls its render
     * function.
     */

    class Windowlet {
        // An individual windowlets title, and the vector of its modules. This has to be a vector, since
        // the order of the modules matters. If you use a set, the modules will get ordered by the value
        // of their pointer, which means the exact order of modules is indeterminate.
        const std::string title;
        const std::vector<WModule*> modules;

    public:
        // Windowlet constructor. Takes the windowlet title and its modules
        explicit Windowlet(std::string title, std::vector<WModule*>&& modules);
        virtual ~Windowlet();

        // The individual windowlets renderer
        virtual void render();
    };

} // namespace LRI::RCI

#endif // WINDOWLET_H
