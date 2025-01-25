#ifndef TARGETCHOOSER_H
#define TARGETCHOOSER_H

#include <string>
#include <vector>
#include <interfaces/COMPort.h>

#include "nlohmann/json.hpp"

#include "BaseUI.h"
#include "interfaces/RCP_Interface.h"

namespace LRI::RCI {
    class TargetChooser final : public BaseUI {
        class InterfaceChooser;

        static TargetChooser* instance;

        RCP_Interface* interf;
        int pollingRate;
        InterfaceChooser* chooser;
        std::vector<std::string> targetpaths;
        size_t chosenConfig;

        std::vector<std::string> interfaceoptions;
        size_t chosenInterface;

        nlohmann::json targetconfig;

        TargetChooser();
        void initWindows();

        class InterfaceChooser {
        protected:
            TargetChooser* targetchooser;

        public:
            explicit InterfaceChooser(TargetChooser* targetchooser);
            virtual RCP_Interface* render() = 0;
            virtual ~InterfaceChooser() = default;
        };

        class COMPortChooser final : public InterfaceChooser {
            bool enumSerialDevs();
            std::vector<std::string> portlist;
            size_t selectedPort;
            bool error;
            int baud;
            COMPort* port;

        public:
            explicit COMPortChooser(TargetChooser* targetchooser);
            RCP_Interface* render() override;
        };

        class VirtualPortChooser final : public InterfaceChooser {
        public:
            VirtualPortChooser();
            RCP_Interface* render() override;
        };

    public:
        void render() override;
        static TargetChooser* getInstance();
        [[nodiscard]] const RCP_Interface* getInterface() const;

        void hideWindow() override;
        void showWindow() override;

        ~TargetChooser() override = default;
    };
}

#endif //TARGETCHOOSER_H
