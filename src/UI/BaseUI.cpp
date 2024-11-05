#include "UI/BaseUI.h"
#include <vector>

namespace LRI::RCI {
    std::vector<BaseUI*> windows;

    BaseUI::BaseUI() {
        windows.push_back(this);
    }

    void renderWindows() {
        for(const auto& window : windows) {
            window->render();

        }
    }
}
