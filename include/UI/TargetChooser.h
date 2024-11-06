#ifndef TARAGETCHOOSER_H
#define TARGETCHOOSER_H

#include <string>
#include <vector>

#include "BaseUI.h"
#include "devices/RCP_Interface.h"

namespace LRI::RCI {
    class TargetChooser : public BaseUI {
        static TargetChooser* instance;
        std::vector<std::string> portlist;
        size_t selectedPort;
        RCP_Interface* port;

        bool enumDevs();
        TargetChooser();

    public:
        void render() override;
        static const TargetChooser* getInstance();
        const RCP_Interface* getInterface() const;
    };
}

#endif //TARAGETCHOOSER_H
