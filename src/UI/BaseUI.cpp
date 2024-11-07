#include "UI/BaseUI.h"
#include <set>

namespace LRI::RCI {
    std::set<BaseUI*> windows;

    BaseUI::BaseUI() {
        windows.insert(this);
    }

    void BaseUI::destroy() {
        windows.erase(this);
    }

    void renderWindows() {
        for(const auto& window : windows) {
            window->render();
        }
    }
}
