#if defined(_WIN32)
// Make sure Windows doesn't allocate a console window, since we have the UI
#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")

/*
 * Even though Windows.h is not explicitly used in this file, one of the macros it defines is not checked if it has
 * already been defined (silly windows) so it conflicts with when GLFW also defines it. However, glfw is smart and
 * does check if this particular macro has already been defined, so Windows.h must be included first that way the smart
 * people avoid redefining a macro and raising a compiler warning and the dumb people can do what they want.
 */
#include <Windows.h>
#endif

#include "GLFW/glfw3.h"

#include "UI/Splash.h"
#include "UI/Window.h"
#include "UI/gutils.h"

/*
 * This is the main file for RCI. See Windowlet.h for more information on program structure
 */

// A very small main function :)
int main() {
    // Initialize glfw
    if(!glfwInit()) {
        return -1;
    }

    // Set up shared GPU resources like fonts and the bird logo
    LRI::RCI::style::setup();

    // Show the splash screen for style points and to load a few other things
    {
        LRI::RCI::Splash s;
        s.show();
    }

    // Main application loop
    {
        LRI::RCI::Window w;
        w.show();
    }

    // Clean up GPU resources
    LRI::RCI::style::cleanup();

    glfwTerminate();

    return 0;
}
