#include "UI/BaseUI.h"
#include <set>

namespace LRI::RCI {
    std::set<BaseUI*> windows;

    BaseUI::BaseUI() {}

    void BaseUI::showWindow() {
        if(!windows.contains(this)) windows.insert(this);
    }

    void BaseUI::hideWindow() {
        if(windows.contains(this)) windows.erase(this);
    }

    void renderWindows() {
        for(const auto& window : windows) {
            window->render();
        }
    }
}
