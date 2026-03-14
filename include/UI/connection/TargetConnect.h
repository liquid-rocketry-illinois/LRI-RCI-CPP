#ifndef LRI_CONTROL_PANEL_TARGETCONNECT_H
#define LRI_CONTROL_PANEL_TARGETCONNECT_H

#include "UI/positioning.h"
#include "interfaces/RCP_Interface.h"

namespace LRI::RCI::TargetConnect {
    RCP_Interface* render(const Box& region);
}

#endif // LRI_CONTROL_PANEL_TARGETCONNECT_H
