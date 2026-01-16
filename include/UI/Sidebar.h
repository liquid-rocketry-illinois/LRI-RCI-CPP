#ifndef LRI_CONTROL_PANEL_SIDEBAR_H
#define LRI_CONTROL_PANEL_SIDEBAR_H

namespace LRI::RCI {
    enum class SideBarOptions { TARGET_VIEW, HDF_VIEW, CONFIG_EDITOR };

    namespace Sidebar {
        SideBarOptions render();
    }
} // namespace LRI::RCI

#endif // LRI_CONTROL_PANEL_SIDEBAR_H
