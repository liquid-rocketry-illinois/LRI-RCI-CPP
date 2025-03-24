// Make sure Windows doesn't allocate a console window, since we have the UI
#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")

// Even though Windows.h is not explicitly used in this file, one of the macros it defines is not checked if it has
// already been defined (silly windows) so it conflicts with when GLFW also defines it. However, glfw is smart and
// does check if this particular macro has already been defined, so Windows.h must be included first that way the smart
// people avoid redefining a macro and raising a compiler warning and the dumb people can do what they want.
#include <Windows.h>

#include "GLFW/glfw3.h"

#include "utils.h"

#include "hardware/Sensors.h"
#include "hardware/TestState.h"
#include "UI/Windowlet.h"

// A very small main function :)
int main() {
    // Initialize glfw and create the window
    if(!glfwInit()) {
        return -1;
    }

    glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
    GLFWwindow* window = glfwCreateWindow(1920, 1080, "LRI RCI", nullptr, nullptr);
    if(!window) {
        return -1;
    }

    // Run the init function in utils.cpp
    LRI::RCI::imgui_init(window);

    // A very simple loop :)
    // While the window should not close, render stuff
    while(!glfwWindowShouldClose(window)) {
        LRI::RCI::imgui_prerender(window);
        LRI::RCI::Windowlet::renderWindowlets();
        LRI::RCI::imgui_postrender(window);

        // Update hardware that needs updating
        LRI::RCI::TestState::getInstance()->update(); // heartbeats
        LRI::RCI::Sensors::getInstance()->update(); // Serialization threads
    }

    // Once the window should close, then terminate libraries
    LRI::RCI::imgui_shutdown(window);

    return 0;
}
