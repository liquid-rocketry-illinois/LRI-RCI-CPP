#include "UI/Windowlet.h"

#include <UI/TargetChooser.h>

#include <utility>

#include <iostream>
#include "UI/TestStateViewer.h"

namespace LRI::RCI {
    // The global windowlet set
    std::set<Windowlet*> Windowlet::windows;

    // The anti-spam button timer
    StopWatch WModule::buttonTimer = StopWatch();
    int WModule::CLASSID = 0;

    WModule::WModule() : classid(CLASSID++) {}

    void Windowlet::renderWindowlets() {
        // Important that this line is first, since the Control Window can modify the list of
        // windows between frames. Not good.
        for(auto* w : windows) w->render();
        ControlWindowlet::getInstance()->render();
    }

    Windowlet::Windowlet(std::string title, const std::vector<WModule*>& modules, bool addToSet) :
        title(std::move(title)), modules(modules) {
        if(addToSet) windows.insert(this);
    }

    Windowlet::Windowlet(std::string title, const std::vector<WModule*>& modules) :
        Windowlet(std::move(title), modules, true) {}

    Windowlet::~Windowlet() {
        windows.erase(this);
        for(const auto* mod : modules) delete mod;
    }

    void Windowlet::render() {
        size_t size = modules.size() - 1;
        size_t i = 0;

        if(ImGui::Begin(title.c_str()))
            for(auto* mod : modules) {
                mod->render();
                if(i++ != size) ImGui::Separator();
            }
        ImGui::End();
    }

    // TODO: target chooser module here
    ControlWindowlet::ControlWindowlet() :
        Windowlet("Target Selector", std::vector{static_cast<WModule*>(new TargetChooser(this))}, false) {}

    void ControlWindowlet::cleanup() {
        ImGui::SaveIniSettingsToDisk(inipath.c_str());
        std::set w(windows);
        for(const auto* win : w) delete win;
    }

    ControlWindowlet* ControlWindowlet::getInstance() {
        static ControlWindowlet* instance = nullptr;
        if(instance == nullptr) instance = new ControlWindowlet();
        return instance;
    }

    void ControlWindowlet::render() {
        ImGui::SetNextWindowPos(scale(ImVec2(37.5, 50)), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(scale(ImVec2(550, 225)), ImGuiCond_FirstUseEver);

        size_t size = modules.size() - 1;
        size_t i = 0;
        if(ImGui::Begin(title.c_str(), nullptr, ImGuiWindowFlags_NoDocking))
            for(auto* mod : modules) {
                mod->render();
                if(i++ != size) ImGui::Separator();
            }
        ImGui::End();
    }
} // namespace LRI::RCI
