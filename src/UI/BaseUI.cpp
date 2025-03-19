#include "UI/BaseUI.h"
#include <set>

namespace LRI::RCI {
    std::set<BaseUI*> BaseUI::windows;
    std::set<BaseUI*> BaseUI::activeWindows;
    StopWatch BaseUI::buttonTimer;

    BaseUI::BaseUI() {
        windows.insert(this);
        activeWindows.insert(this);
    }

    BaseUI::~BaseUI() {
        windows.erase(this);
        if(activeWindows.contains(this)) activeWindows.erase(this);
    }

    // Showing and hiding windows just consists of adding and removing the windows from the windows set
    void BaseUI::showWindow() {
        if(!activeWindows.contains(this)) activeWindows.insert(this);
    }

    void BaseUI::hideWindow() {
        if(activeWindows.contains(this)) activeWindows.erase(this);
    }

    // Rendering/updating just consists of iterating through the active windows and calling their render function
    void BaseUI::renderWindows() {
        for(const auto& window : activeWindows) {
            window->render();
        }
    }

    void BaseUI::reset() {
    }

    // A copy of the set is made so that as we iterate we dont mess up the set the pointers are being removed from
    void BaseUI::hideAll() {
        for(const std::set wins(activeWindows); const auto& window : wins) window->hideWindow();
    }
}
