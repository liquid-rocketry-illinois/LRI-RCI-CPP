#ifndef LRI_CONTROL_PANEL_SIDEBAR_H
#define LRI_CONTROL_PANEL_SIDEBAR_H

#include "positioning.h"

namespace LRI::RCI {
    enum class SideBarOptions { NONE, CONNECT, OVERVIEW, PID, CONFIG, HDF, PACKETB, PACKETI };

    namespace Sidebar {
        SideBarOptions render(const Box& region);
    }
} // namespace LRI::RCI

#endif // LRI_CONTROL_PANEL_SIDEBAR_H
