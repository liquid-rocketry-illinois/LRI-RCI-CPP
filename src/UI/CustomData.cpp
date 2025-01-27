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

            // Button to change interpretation mode
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

                // If the button is pushed the display stream needs to be cleared and reformatted
                display.str("");
                formatRaw(display, raw, mode, &numElems);

                // The buffer for user output is also cleared since the format can change
                memset(out, 0, OUT_SIZE);
            }

            // Clearing the serial output just involves clearing the two streams
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
                // Create the "exports" directory if it does not already exist. If it exists as a file, do nothing
                if(std::filesystem::exists("exports")) {
                    if(!std::filesystem::is_directory("./exports")) {
                        return;
                    }
                }

                else std::filesystem::create_directory("./exports");

                const auto now = std::chrono::system_clock::now();
                std::ofstream file(std::format("./exports/{:%d-%m-%Y-%H-%M-%OS}-serial", now));

                // formatRaw can fortunately also be used on an ofstream
                formatRaw(file, raw, mode);
            }

            ImGui::Separator();

            // An imgui child to contain the text
            if(ImGui::BeginChild("##serialchild", scale(ImVec2(540, 175)))) {
                // Displaying the text just by rendering the string stream
                ImGui::TextUnformatted(display.str().c_str());
                ImGui::EndChild();
            }

            ImGui::Separator();

            // Section for user content
            bool lockButtons = buttonTimer.timeSince() < BUTTON_DELAY;
            if(lockButtons) ImGui::BeginDisabled();

            // The input field is different depending on the interpret mode
            switch(mode) {
                case InterpretMode::DEC: {
                    ImGui::SetNextItemWidth(scale(100));
                    bool send = ImGui::InputText("##serialsend", out, 4, ImGuiInputTextFlags_EnterReturnsTrue);
                    ImGui::SameLine();
                    send = send || ImGui::Button("Send");

                    if(send) {
                        // Parse number from string stored in out
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
                        // Parse int from string with base of 16
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
                        // Sending the string is much easier
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
        // Parse data differently depending on interpret mode. See reformatRaw for more info
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
        // Clear both raw and display
        raw.clear();
        display.str("");

        // Reset other variables to defaults
        numElems = 1;
        mode = InterpretMode::STR;
    }

    // Helper function to take raw data and format it according to the current interpret mode
    void CustomData::formatRaw(std::basic_ostream<char>& out, std::vector<uint8_t>& raw,
                               const InterpretMode& mode, int* elems) {
        // A numElems can be passed in if desired
        int* numElems;
        if(elems != nullptr) {
            numElems = elems;
            *numElems = 1;
        }
        else numElems = new int(1);

        switch(mode) {

            // If the mode is hexadecimal or decimal we do basically the same in both, just slight formatting
            // differences of the actual character
            case InterpretMode::HEX:
            case InterpretMode::DEC: {
                for(uint8_t c : raw) {
                    // If hex, format to 2 digits of uppercase hex padded with zeros
                    if(mode == InterpretMode::HEX) out << std::format("{:02X}", c) << " ";
                    else out << std::format("{:03}", c) << " ";
                    // If decimal, format to 3 digits padded with zeros

                    // This will group numbers into 2 groups of 8, on one line. Each line will be in the format of:
                    // 00 00 00 00 00 00 00 00    00 00 00 00 00 00 00 00
                    // and repeat. 3 digits for decimal
                    if(*numElems % 16 == 0) out << "\n";
                    else if(*numElems % 8 == 0) out << "   ";
                    *numElems = ((*numElems) + 1) % 16;
                }
                break;
            }

            case InterpretMode::STR: {
                // I have not found a way to insert an entire uint8_t vector into a string stream nicely, so a loop it is
                for(uint8_t c : raw) out << (char) c;
                break;
            }
        }

        if(elems == nullptr) delete numElems;
    }
}
