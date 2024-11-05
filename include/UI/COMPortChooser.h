#ifndef COMPORTCHOOSER_H
#define COMPORTCHOOSER_H

#include <string>
#include <vector>

#include "BaseUI.h"
#include "devices/COMPort.h"

namespace LRI::RCI {
    class COMPortChooser : public BaseUI {
        static COMPortChooser* instance;
        std::vector<std::string> portlist;
        size_t selectedPort;
        COMPort* port;

        bool enumDevs();
        COMPortChooser();

    public:
        void render() override;
        static const COMPortChooser* getInstance();
        const COMPort* getComPort() const;
    };
}

#endif //COMPORTCHOOSER_H
