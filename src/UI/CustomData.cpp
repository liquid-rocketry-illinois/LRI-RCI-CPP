#include "UI/CustomData.h"
#include <iomanip>
#include <filesystem>
#include <fstream>

namespace LRI::RCI {
    CustomData* CustomData::instance;

    CustomData* CustomData::getInstance() {
        if(instance == nullptr) instance = new CustomData();
        return instance;
    }

    std::string CustomData::interpretModeToString(InterpretMode mode) {
        switch(mode) {
            case InterpretMode::HEX:
                return "HEX";

            case InterpretMode::DEC:
                return "DEC";

            case InterpretMode::STR:
                return "TEXT";

            default:
                return "";
        }
    }

    CustomData::CustomData() : raw(), display(), mode(InterpretMode::STR), out(), numElems(1) {
    }

    void CustomData::render() {
        ImGui::SetNextWindowPos(scale(ImVec2(50, 525)), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(scale(ImVec2(550, 250)), ImGuiCond_FirstUseEver);

        if(ImGui::Begin("Custom Data (serial monitor)", nullptr, ImGuiWindowFlags_NoResize)) {
            ImGui::Text("Interpret Mode: ");
            ImGui::SameLine();
            if(ImGui::Button((interpretModeToString(mode) + "##intmodebtn").c_str())) {
                switch(mode) {
                    case InterpretMode::HEX:
                        mode = InterpretMode::DEC;
                        break;

                    case InterpretMode::DEC:
                        mode = InterpretMode::STR;
                        break;

                    case InterpretMode::STR:
                        mode = InterpretMode::HEX;
                        break;
                }

                display.str("");
                formatRaw(display, raw, mode, &numElems);
                memset(out, 0, OUT_SIZE);
            }

            ImGui::SameLine();
            ImGui::Text("|");
            ImGui::SameLine();
            if(ImGui::Button("Clear##serialclear")) {
                display.str("");
                raw.clear();
            }

            ImGui::SameLine();
            ImGui::Text("|");
            ImGui::SameLine();
            if(ImGui::Button("Save to file")) {
                if(std::filesystem::exists("exports")) {
                    if(!std::filesystem::is_directory("./exports")) {
                        return;
                    }
                }

                else std::filesystem::create_directory("./exports");

                const auto now = std::chrono::system_clock::now();
                std::ofstream file(std::format("./exports/{:%d-%m-%Y-%H-%M-%OS}-serial", now));
                formatRaw(file, raw, mode);
            }

            ImGui::Separator();

            if(ImGui::BeginChild("##serialchild", scale(ImVec2(540, 175)))) {
                ImGui::TextUnformatted(display.str().c_str());
                ImGui::EndChild();
            }

            ImGui::Separator();

            bool lockButtons = buttonTimer.timeSince() < BUTTON_DELAY;
            if(lockButtons) ImGui::BeginDisabled();
            switch(mode) {
                case InterpretMode::DEC: {
                    ImGui::SetNextItemWidth(scale(100));
                    bool send = ImGui::InputText("##serialsend", out, 4, ImGuiInputTextFlags_EnterReturnsTrue);
                    ImGui::SameLine();
                    send = send || ImGui::Button("Send");

                    if(send) {
                        int val = std::stoi(out);
                        if(val > 255 || val < 0) break;
                        uint8_t rawval = static_cast<uint8_t>(val);
                        RCP_sendRawSerial(&rawval, 1);
                        buttonTimer.reset();
                    }
                    break;
                }

                case InterpretMode::HEX: {
                    ImGui::SetNextItemWidth(scale(100));
                    bool send = ImGui::InputText("##serialsend", out, 3, ImGuiInputTextFlags_EnterReturnsTrue);
                    ImGui::SameLine();
                    send = send || ImGui::Button("Send");

                    if(send) {
                        int val = std::stoi(out, nullptr, 16);
                        if(val > 255 || val < 0) break;
                        uint8_t rawval = static_cast<uint8_t>(val);
                        RCP_sendRawSerial(&rawval, 1);
                        buttonTimer.reset();
                    }
                    break;
                }

                case InterpretMode::STR: {
                    ImGui::SetNextItemWidth(scale(485));
                    bool send = ImGui::InputText("##serialsend", out, OUT_SIZE, ImGuiInputTextFlags_EnterReturnsTrue);
                    ImGui::SameLine();
                    send = send || ImGui::Button("Send");

                    if(send) {
                        std::string str(out);
                        if(str.length() > 63) break;
                        RCP_sendRawSerial(reinterpret_cast<const uint8_t*>(str.c_str()), str.length());
                        memset(out, 0, OUT_SIZE);
                        buttonTimer.reset();
                    }

                    break;
                }
            }

            if(lockButtons) ImGui::EndDisabled();

            ImGui::End();
        }
    }

    void CustomData::recevieRCPUpdate(const RCP_CustomData& data) {
        switch(mode) {
            case InterpretMode::HEX:
            case InterpretMode::DEC: {
                for(int i = 0; i < data.length; i++) {
                    if(mode == InterpretMode::HEX) display << std::format("{:02X}", ((uint8_t*) data.data)[i]) << " ";
                    else display << std::format("{:03}", ((uint8_t*) data.data)[i]) << " ";

                    if(numElems % 16 == 0) display << "\n";
                    else if(numElems % 8 == 0) display << "   ";
                    numElems = (numElems + 1) % 16;
                }
                break;
            }

            case InterpretMode::STR: {
                for(int i = 0; i < data.length; i++) {
                    display << static_cast<char*>(data.data)[i];
                }
            }
        }
        raw.insert(raw.end(), static_cast<uint8_t*>(data.data), static_cast<uint8_t*>(data.data) + data.length);
    }

    void CustomData::reset() {
        raw.clear();
        display.str("");
        numElems = 1;
        mode = InterpretMode::STR;
    }

    void CustomData::formatRaw(std::basic_ostream<char>& out, std::vector<uint8_t>& raw,
                               const InterpretMode& mode, int* elems) {
        int* numElems;
        if(elems != nullptr) {
            numElems = elems;
            *numElems = 1;
        }
        else numElems = new int(1);

        switch(mode) {
            case InterpretMode::HEX:
            case InterpretMode::DEC: {
                for(uint8_t c : raw) {
                    if(mode == InterpretMode::HEX) out << std::format("{:02X}", c) << " ";
                    else out << std::format("{:03}", c) << " ";

                    if(*numElems % 16 == 0) out << "\n";
                    else if(*numElems % 8 == 0) out << "   ";
                    *numElems = ((*numElems) + 1) % 16;
                }
                break;
            }

            case InterpretMode::STR: {
                for(uint8_t c : raw) {
                    out << (char) c;
                }
                break;
            }
        }

        if(elems == nullptr) delete numElems;
    }
}
