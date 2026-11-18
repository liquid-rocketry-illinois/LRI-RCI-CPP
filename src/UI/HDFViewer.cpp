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
            const std::vector<const HardwareQualifier*> sortedQuals;

            static std::vector<const HardwareQualifier*> makeSortedQuals(const std::set<HardwareQualifier>& quals) {
                std::vector<const HardwareQualifier*> squals;
                for(const auto& q : quals) squals.push_back(&q);
                std::ranges::sort(squals, [](auto& l, auto& r) { return *l < *r; });
                return squals;
            }

        public:
            explicit HDFViewerWM(const EventLog* elog) :
                elog(elog), sortedQuals(makeSortedQuals(elog->getRegisteredQuals())) {}
            ~HDFViewerWM() override { delete elog; }

            void render() override {
                ImGui::PushStyleColor(ImGuiCol_TableBorderLight, colors::TEXT);
                SCOPE_EXIT { ImGui::PopStyleColor(); };

                if(!ImGui::BeginTable("##hdfviewertable", 2,
                                      ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_RowBg |
                                          ImGuiTableFlags_SizingFixedFit,
                                      ImVec2{0, ImGui::GetWindowSize().y - 10_sc}))
                    return;
                SCOPE_EXIT { ImGui::EndTable(); };

                ImGui::TableSetupColumn("##left", ImGuiTableColumnFlags_NoResize, 300_sc);
                ImGui::TableSetupColumn("##right", ImGuiTableColumnFlags_NoResize, ImGui::GetWindowSize().x - 300_sc);

                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);

                if(ImGui::TreeNode("System")) {
                    ImGui::Selectable("Test state information");
                    ImGui::Selectable("Log");
                    ImGui::Selectable("Prompts");
                    ImGui::TreePop();
                }

                RCP_DeviceClass prevclass = RCP_DEVCLASS_AMALGAMATE;
                bool treeNodeOpen = false;

                for(const auto& qual : sortedQuals) {
                    if(qual->devclass != prevclass) {
                        if(treeNodeOpen) ImGui::TreePop();
                        prevclass = qual->devclass;
                        treeNodeOpen = ImGui::TreeNode(devclassToString(qual->devclass).c_str());
                    }

                    if(!treeNodeOpen) continue;

                    ImGui::Selectable(qual->name.c_str());
                }

                if(treeNodeOpen) ImGui::TreePop();

                ImGui::TableSetColumnIndex(1);
                ImGui::Text("Col 1");
            }
        };

    public:
        explicit HDFViewer(const EventLog* elog) : Windowlet("Event Log View", {new HDFViewerWM(elog)}) {}
        ~HDFViewer() override = default;

        void render() override {
            ImVec2 wsize = ImGui::GetMainViewport()->Size;
            ImVec2 wpos = ImGui::GetMainViewport()->Pos;
            ImGui::SetNextWindowSize(wsize - ImVec2{50_sc, 75_sc});
            ImGui::SetNextWindowPos(wpos + ImVec2{25_sc, 50_sc});
            ImGui::SetNextWindowViewport(ImGui::GetMainViewport()->ID);

            Windowlet::render(nullptr,
                              ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoDecoration);
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
