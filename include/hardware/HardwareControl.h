#ifndef LRI_CONTROL_PANEL_HARDWARECONTROL_H
#define LRI_CONTROL_PANEL_HARDWARECONTROL_H

#include <set>

#include "HardwareQualifier.h"
#include "interfaces/RCP_Interface.h"

namespace LRI::RCI::HWCTRL {
    extern int POLLS_PER_UPDATE;

    void start(RCP_Interface* interf);
    void update();
    void pause();
    void end();

    void setHardwareConfig(const std::set<HardwareQualifier>& quals);
} // namespace LRI::RCI::HWCTRL

#endif // LRI_CONTROL_PANEL_HARDWARECONTROL_H
