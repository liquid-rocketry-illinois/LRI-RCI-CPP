#include "UI/Windowlet.h"

#include <UI/TargetChooser.h>

#include <utility>

#include "UI/TestStateViewer.h"
#include <iostream>

namespace LRI::RCI {
    std::set<Windowlet*> Windowlet::windows;
    StopWatch WModule::buttonTimer = StopWatch();

    void Windowlet::renderWindowlets() {
        ControlWindowlet::getInstance()->render();
        for(auto* w : windows) {
            w->render();
        }
    }

    Windowlet::Windowlet(std::string title, const std::vector<WModule*>& modules, bool addToSet) : title(std::move(title)),
        modules(modules) {
        if(addToSet) windows.insert(this);
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
    ControlWindowlet::ControlWindowlet() :
        Windowlet("Target Selector", std::vector{static_cast<WModule*>(new TargetChooser(this))}, false),
        interf(nullptr) {
    }

    void ControlWindowlet::cleanup() {
        std::set<Windowlet*> w(windows);
        for(const auto* win : w) delete win;
        interf = nullptr;
    }

    ControlWindowlet* ControlWindowlet::getInstance() {
        static ControlWindowlet* instance = nullptr;
        if(instance == nullptr) instance = new ControlWindowlet();
        return instance;
    }

    void ControlWindowlet::render() {
        ImGui::SetNextWindowPos(scale(ImVec2(50, 50)), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(scale(ImVec2(550, 225)), ImGuiCond_FirstUseEver);
        if(ImGui::Begin(title.c_str(), nullptr, ImGuiWindowFlags_NoDocking))
            for(auto* mod : modules) mod->render();
        ImGui::End();
    }

    RCP_Interface* ControlWindowlet::getInterf() const {
        return interf;
    }
}
