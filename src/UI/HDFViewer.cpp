#include "UI/HDFViewer.h"
#include "UI/gutils.h"

namespace {
    using namespace LRI::RCI;

    class HDFPickerWM : public WModule {
        Window* const owner;

    public:
        explicit HDFPickerWM(Window* owner) : owner(owner) {}
        ~HDFPickerWM() override = default;

        void render() override {
            ImGui::PushID("HDFPicker");
            ImGui::PushID(classid);

            SCOPE_EXIT {
                ImGui::PopID();
                ImGui::PopID();
            };

            ImGui::Text("Examine saved RCI test logs");
            if(ImGui::Button("Browse")) {}
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
