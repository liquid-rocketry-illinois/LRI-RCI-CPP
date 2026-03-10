#ifndef LRI_CONTROL_PANEL_SIDEBAR_H
#define LRI_CONTROL_PANEL_SIDEBAR_H

namespace LRI::RCI {
    enum class SideBarOptions { NONE, CONNECT, OVERVIEW, PID, CONFIG, HDF, PACKETB, PACKETI };

    namespace Sidebar {
        SideBarOptions render();
    }
} // namespace LRI::RCI

#endif // LRI_CONTROL_PANEL_SIDEBAR_H
