#include "UI/HDFViewer.h"

#include "UI/gutils.h"

#include <format>
#include "nfd_helper.h"
#include "hardware/EventLog.h"

namespace {
    using namespace LRI::RCI;

    class HDFPickerWM : public WModule {
        Window* const owner;

    public:
        explicit HDFPickerWM(Window* owner) : owner(owner) {}
        ~HDFPickerWM() override = default;

        std::string msg;

        void render() override {
            ImGui::PushID("HDFPicker");
            ImGui::PushID(classid);

            SCOPE_EXIT {
                ImGui::PopID();
                ImGui::PopID();
            };

            ImGui::Text("Examine saved RCI test logs");
            if(ImGui::Button("Browse")) {
                FilePickResult fpr = pickRCIFile(owner);

                if(fpr.state == FilePickResult::SUCCESS) {
                    try {
                        EventLog* elog = new EventLog(fpr.outpath);
                        msg = "success!";
                        delete elog;
                    }

                    catch(const std::runtime_error& e) {
                        msg = std::string("Error opening RCI log: ") + e.what();
                    }

                }

                else if(fpr.state == FilePickResult::CANCELLED) {
                    msg = "";
                }

                else {
                    msg = "Internal error. Retry.";
                }
            }

            ImGui::Text("%s", msg.c_str());
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
