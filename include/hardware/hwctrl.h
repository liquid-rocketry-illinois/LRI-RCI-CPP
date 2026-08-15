#ifndef LRI_CONTROL_PANEL_HWCTRL_H
#define LRI_CONTROL_PANEL_HWCTRL_H

#include "interfaces/RCP_Interface.h"

#include "hardware/json.h"

namespace LRI::RCI::hwctrl {
    void start(RCP_Interface* interf, const TargetConfig& config);
    void update();
    void end();

    const std::string& interfName();
    size_t interfBytesWaiting();
    bool isOpen();
}

#endif // LRI_CONTROL_PANEL_HWCTRL_H
