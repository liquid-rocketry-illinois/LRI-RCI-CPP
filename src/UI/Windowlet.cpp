#include "UI/Windowlet.h"

#include "UI/Window.h"
#include "UI/gutils.h"

namespace LRI::RCI {
    int WModule::CLASSID = 0;
    StopWatch WModule::buttonTimer;

    WModule::WModule() : classid(CLASSID++) {}

    Windowlet::Windowlet(std::string title, std::vector<WModule*>&& modules) :
        title(std::move(title)), modules(modules) {}

    Windowlet::~Windowlet() {
        for(const auto* mod : modules) delete mod;
    }

    void Windowlet::render(bool* closed, ImGuiWindowFlags flags) {
        size_t size = modules.size() - 1;
        size_t i = 0;

        if(ImGui::Begin(title.c_str(), closed, flags)) {
            style::scaling_factor = ImGui::GetWindowViewport()->DpiScale;
            for(auto* mod : modules) {
                mod->render();
                if(i++ != size) ImGui::Separator();
            }
        }
        ImGui::End();
    }

    void Windowlet::render() {
        render(nullptr, ImGuiWindowFlags_None);
    }

    // TODO: target chooser module here
    // ControlWindowlet::ControlWindowlet() :
    //     Windowlet("Target Selector", std::vector{static_cast<WModule*>(new TargetChooser(this)),
    //     static_cast<WModule*>(new ErrorWindow())}, false) {}
    //
    // ControlWindowlet::~ControlWindowlet() {
    //     ImGui::SaveIniSettingsToDisk(inipath.c_str());
    //     std::set w(windows);
    //     for(const auto* win : w) delete win;
    // }
    //
    // void ControlWindowlet::render() {
    //     ImGui::SetNextWindowPos(ImVec2(37.5_sc, 50_sc), ImGuiCond_FirstUseEver);
    //     ImGui::SetNextWindowSize(ImVec2(550_sc, 225_sc), ImGuiCond_FirstUseEver);
    //     ImGui::SetNextWindowViewport(ImGui::GetMainViewport()->ID);
    //
    //     Windowlet::render();
    // }
} // namespace LRI::RCI
