#ifndef TARAGETCHOOSER_H
#define TARGETCHOOSER_H

#include <string>
#include <vector>

#include "nlohmann/json.hpp"

#include "BaseUI.h"
#include "interfaces/RCP_Interface.h"

namespace LRI::RCI {
    class TargetChooser final : public BaseUI {
        class InterfaceChooser;
        static TargetChooser* instance;

        RCP_Interface* interf;
        InterfaceChooser* chooser;
        std::vector<std::string> targetpaths;
        size_t chosenConfig;

        std::vector<std::string> interfaceoptions;
        size_t chosenInterface;

        nlohmann::json targetconfig;

        TargetChooser();

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

        public:
            explicit COMPortChooser(TargetChooser* targetchooser);
            RCP_Interface* render() override;
        };

    public:
        void render() override;
        static TargetChooser* const getInstance();
        const RCP_Interface* getInterface() const;

        void hideWindow() override;
        void showWindow() override;

        ~TargetChooser() override = default;
    };
}

#endif //TARAGETCHOOSER_H
