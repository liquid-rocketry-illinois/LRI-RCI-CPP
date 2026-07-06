#if !defined(RCIDEBUG)
// Make sure Windows doesn't allocate a console window, since we have the UI
#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")

#endif

/*
 * Even though Windows.h is not explicitly used in this file, one of the macros it defines is not checked if it has
 * already been defined (silly windows) so it conflicts with when GLFW also defines it. However, glfw is smart and
 * does check if this particular macro has already been defined, so Windows.h must be included first that way the smart
 * people avoid redefining a macro and raising a compiler warning and the dumb people can do what they want.
 */
#include <Windows.h>

#include <print>

#include "util/system.h"
#include "GLFW/glfw3.h"
#include "UI/window.h"
#include "util/settings.h"

/*
 * This is the main file for RCI. See Windowlet.h for more information on program structure
 */


// A very small main function :)
int main() {
    // Initialize glfw and create the window
    if(!glfwInit()) {
        std::println("Failed to initialize GLFW");
        return -1;
    }

    // Will loop until splash finishes
    LRI::RCI::splash::show();

    // Will loop until user exits
    LRI::RCI::show();

    glfwTerminate();

    LRI::RCI::settings::writeSettings();
    return 0;
}
