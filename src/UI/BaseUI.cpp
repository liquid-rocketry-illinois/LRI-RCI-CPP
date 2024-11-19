#include "UI/BaseUI.h"
#include <set>

namespace LRI::RCI {
    std::set<BaseUI*> BaseUI::windows;
    StopWatch BaseUI::buttonTimer;

    void BaseUI::showWindow() {
        if(!windows.contains(this)) windows.insert(this);
    }

    void BaseUI::hideWindow() {
        if(windows.contains(this)) windows.erase(this);
    }

    void BaseUI::renderWindows() {
        for(const auto& window : windows) {
            window->render();
        }
    }

    void BaseUI::hideAll() {
        for(const auto& window : windows) window->hideWindow();
    }

}
