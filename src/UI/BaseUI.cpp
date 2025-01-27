#include "UI/BaseUI.h"
#include <set>

namespace LRI::RCI {
    std::set<BaseUI*> BaseUI::windows;
    StopWatch BaseUI::buttonTimer;

    // Showing and hiding windows just consists of adding and removing the windows from the windows set
    void BaseUI::showWindow() {
        if(!windows.contains(this)) windows.insert(this);
    }

    void BaseUI::hideWindow() {
        if(windows.contains(this)) windows.erase(this);
    }

    // Rendering/updating just consists of iterating through the active windows and calling their render function
    void BaseUI::renderWindows() {
        for(const auto& window : windows) {
            window->render();
        }
    }

    void BaseUI::reset() {}

    // A copy of the set is made so that as we iterate we dont mess up the set the pointers are being removed from
    void BaseUI::hideAll() {
        std::set<BaseUI*> wins(windows);
        for(const auto& window : wins) window->hideWindow();
    }

}
