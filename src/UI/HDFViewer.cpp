#include "UI/HDFViewer.h"

#include "UI/gutils.h"

#include <format>
#include "hardware/EventLog.h"
#include "nfd_helper.h"

namespace {
    using namespace LRI::RCI;

    class HDFViewerWTF : public WindowTitlebarFunctions {
        Window* const owner;
        const std::string name;

    public:
        HDFViewerWTF(Window* owner, std::string name) : owner(owner), name(std::move(name)) {}
        ~HDFViewerWTF() override = default;

        void extraTitlebarElements(float textY) override {
            ImGui::SetCursorPosY(textY);
            ImGui::Text("%s", name.c_str());
        }

        void menuPopupElements() override {
            if(ImGui::Button("Close")) owner->resetWindowlets();
        }
    };

    class HDFViewer : public Windowlet {
        class HDFViewerWM : public WModule {
            const EventLog* const elog;

        public:
            explicit HDFViewerWM(const EventLog* elog) : elog(elog) {}
            ~HDFViewerWM() override {
                delete elog;
            }

            void render() override { ImGui::Text("Hello!"); }
        };

    public:
        explicit HDFViewer(const EventLog* elog) : Windowlet("Event Log View", {new HDFViewerWM(elog)}) {}
        ~HDFViewer() override = default;

        void render() override {
            ImGui::SetNextWindowSize({550_sc, 225_sc}, ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowPos({50_sc, 50_sc}, ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowViewport(ImGui::GetMainViewport()->ID);
            Windowlet::render();
        }
    };

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
                        owner->setWindowlets(new HDFViewerWTF(owner, "Log Viewer"), {new HDFViewer(elog)});
                        msg = "";
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
