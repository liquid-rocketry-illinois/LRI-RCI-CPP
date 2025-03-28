#include "UI/Windowlet.h"

#include "UI/TestStateViewer.h"

namespace LRI::RCI {
    std::set<Windowlet*> Windowlet::windows;
    StopWatch WModule::buttonTimer = StopWatch();

    void Windowlet::renderWindowlets() {
        for(auto* w : windows) {
            w->render();
        }
    }

    Windowlet::Windowlet(const std::string& title, const std::set<WModule*>& modules) : title(title), modules(modules) {
        windows.insert(this);
    }

    Windowlet::~Windowlet() {
        windows.erase(this);
        for(const auto* mod : modules) delete mod;
    }

    void Windowlet::render() {
        if(ImGui::Begin(title.c_str()))
            for(auto* mod : modules) mod->render();
        ImGui::End();
    }

    // TODO: target chooser module here
    ControlWindowlet::ControlWindowlet() : Windowlet("Target Selector", std::set<WModule*>()) {
    }

    ControlWindowlet* ControlWindowlet::getInstance() {
        static ControlWindowlet* instance = nullptr;
        if(instance == nullptr) instance = new ControlWindowlet();
        return instance;
    }

    void ControlWindowlet::render() {
        if(ImGui::Begin(title.c_str(), nullptr, ImGuiWindowFlags_NoDocking))
            for(auto* mod : modules) mod->render();
        ImGui::End();
    }
}
