#include "UI/HDFViewer.h"

#include "UI/gutils.h"
#include "nfd.h"

#include "nfd_helper.h"
#include <format>

namespace {
    using namespace LRI::RCI;

    class HDFPickerWM : public WModule {
        Window* const owner;

        bool hasResults;
        std::string err;
        std::string path;

    public:
        explicit HDFPickerWM(Window* owner) : owner(owner), hasResults(false) {}
        ~HDFPickerWM() override = default;

        void render() override {
            ImGui::PushID("HDFPicker");
            ImGui::PushID(classid);

            SCOPE_EXIT {
                ImGui::PopID();
                ImGui::PopID();
            };

            ImGui::Text("Examine saved RCI test logs");
            if(ImGui::Button("Browse")) {
                auto res = filePicker(owner);
                hasResults = true;
                if(res.state == FilePickResult::SUCCESS) {
                    err = "success";
                    path = res.outpath.string();
                }

                else if(res.state == FilePickResult::CANCELLED) {
                    err = "cancelled";
                    path = "";
                }

                else {
                    err = std::format("error: {}", NFD_GetError());
                    path = "";
                }
            }

            ImGui::Text("state: %s", err.c_str());
            ImGui::Text("path: %s", path.c_str());
        }
    };
} // namespace

namespace LRI::RCI {
    HDFPicker::HDFPicker(Window* owner) : Windowlet("RCI File Viewer", {new HDFPickerWM(owner)}) {}

    void HDFPicker::render() {
        ImGui::SetNextWindowSize({550_sc, 225_sc}, ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowPos({50_sc, 365_sc}, ImGuiCond_FirstUseEver);

        ImGui::SetNextWindowViewport(ImGui::GetMainViewport()->ID);
        Windowlet::render();
    }
} // namespace LRI::RCI
