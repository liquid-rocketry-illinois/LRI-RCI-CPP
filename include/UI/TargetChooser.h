#ifndef TARGETCHOOSER_H
#define TARGETCHOOSER_H

#include <string>
#include <vector>
#include <interfaces/COMPort.h>

#include "nlohmann/json.hpp"

#include "BaseUI.h"
#include "interfaces/RCP_Interface.h"

namespace LRI::RCI {

    // The most important window. Responsible for initializing and coordinating RCP, the windows, and the interface
    class TargetChooser final : public BaseUI {
        // See below
        class InterfaceChooser;

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

        // In order to easily allow for expansions in the types of interfaces needed, the exact logic for connecting to
        // interfaces is left as another abstraction tree. InterfaceChooser is to interface choosing UI children as
        // BaseUI is to the window classes. The major difference is that in the render/update function, the chooser
        // indicates success by returning a pointer to an open interface
        class InterfaceChooser {
        protected:
            TargetChooser* targetchooser;

        public:
            explicit InterfaceChooser(TargetChooser* targetchooser);

            // The render and update funcion to be overridden
            virtual RCP_Interface* render() = 0;

            virtual ~InterfaceChooser() = default;
        };

        // This is the only actual interface available at the moment. This chooser is for connecting to serial devices
        // (e.g. COM1, COM2, so on). It allows selecting a device and choosing the baud rate
        class COMPortChooser final : public InterfaceChooser {
            // Helper function to enumerate available serial devices and store their names in portlist
            bool enumSerialDevs();

            // Storage for available ports. Ports will be in the format of their handle name, a colon, and the windows
            // display name (ex. COM1:Arduino Serial Device)
            std::vector<std::string> portlist;

            // The index of the selected port
            size_t selectedPort;

            // If there was an error
            bool error;

            // The current baud rate
            int baud;

            // The interface itself
            COMPort* port;

        public:
            explicit COMPortChooser(TargetChooser* targetchooser);
            RCP_Interface* render() override;
        };

        // A testing interface for debugging. See interfaces/VirtualPort.h
        class VirtualPortChooser final : public InterfaceChooser {
        public:
            VirtualPortChooser();
            RCP_Interface* render() override;
        };

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
