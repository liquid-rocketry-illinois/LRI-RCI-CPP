#ifndef TARGETCHOOSER_H
#define TARGETCHOOSER_H

#include <string>
#include <vector>

#include "nlohmann/json.hpp"

#include "BaseUI.h"
#include "interfaces/RCP_Interface.h"

namespace LRI::RCI {
    // In order to easily allow for expansions in the types of interfaces needed, the exact logic for connecting to
    // interfaces is left as another abstraction tree. InterfaceChooser is to interface choosing UI children as
    // BaseUI is to the window classes. The major difference is that in the render/update function, the chooser
    // indicates success by returning a pointer to an open interface
    class InterfaceChooser {
    public:
        explicit InterfaceChooser() = default;

        // The render and update funcion to be overridden
        virtual RCP_Interface* render() = 0;

        virtual ~InterfaceChooser() = default;
    };

    // The most important window. Responsible for initializing and coordinating RCP, the windows, and the interface
    class TargetChooser final : public BaseUI {
        // Singleton instance
        static TargetChooser* instance;

        // The current interface
        RCP_Interface* interf;

        // The rate to call RCP_poll()
        int pollingRate;

        // The current interface chooser
        InterfaceChooser* chooser;

        // A vector to store a list of paths to target files
        std::vector<std::string> targetpaths;

        // Which target configuration has been chosen
        size_t chosenConfig;

        // A list of the interface options
        std::vector<std::string> interfaceoptions;

        // Which interface option has been chosen
        size_t chosenInterface;

        // Stores the parsed target config
        nlohmann::json targetconfig;

        TargetChooser();

        // Helper to initialize all windows with the correct configurations
        void initWindows();

    public:
        // TargetChooser render function
        void render() override;

        // Get singleton instance
        static TargetChooser* getInstance();

        // Get the interface once open
        [[nodiscard]] const RCP_Interface* getInterface() const;

        // Override hide and show functions to replace with empty functions
        void hideWindow() override;
        void showWindow() override;

        ~TargetChooser() override = default;
    };
}

#endif //TARGETCHOOSER_H
