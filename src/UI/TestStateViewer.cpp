#include "UI/TestStateViewer.h"

#include <ranges>

#include "hardware/EStop.h"
#include "hardware/TestState.h"
#include "improgress.h"
#include "utils.h"

// Module for viewing and controlling test state
namespace LRI::RCI {
    TestStateViewer::TestStateViewer() :
        inputHeartbeatRate(0), activeTest(0), dstream(false), doHeartbeats(false), resetTimeOnTestStart(false) {}

    void TestStateViewer::render() {
        ImGui::PushID("TestStateViewer");
        ImGui::PushID(classid);
        if(!TestState::getInited()) ImGui::BeginDisabled();

        ImGui::Text("Test Control");

        bool lockButtons = buttonTimer.timeSince() < BUTTON_DELAY;
        if(lockButtons) ImGui::BeginDisabled();

        // Display controls for starting, stopping, pausing, estopping, and selecting a test
        RCP_TestRunningState state = TestState::getState();

        // For each type of button, they can only be pushed in certain states. The lock variable is reused
        bool lock = state != RCP_TEST_STOPPED;

        // Track if the button is being held for later
        bool pushed = false;

        if(lock) ImGui::BeginDisabled();
        if(ImGui::TimedButton("Start", startTimer)) {
            pushed = true;
            if(startTimer.timeSince() > CONFIRM_HOLD_TIME) {
                TestState::startTest(activeTest);
                buttonTimer.reset();
            }
        }

        if(ImGui::IsItemHovered()) ImGui::SetTooltip("Hold to confirm");
        if(lock) ImGui::EndDisabled();

        lock = state != RCP_TEST_RUNNING && state != RCP_TEST_PAUSED;
        if(lock) ImGui::BeginDisabled();
        ImGui::SameLine();
        if(ImGui::Button("End")) {
            TestState::stopTest();
            buttonTimer.reset();
        }
        if(lock) ImGui::EndDisabled();

        lock = state == RCP_TEST_ESTOP || state == RCP_TEST_STOPPED;
        if(lock) ImGui::BeginDisabled();
        ImGui::SameLine();
        if(ImGui::Button(state == RCP_TEST_PAUSED ? "Resume" : "Pause")) {
            TestState::pause();
            buttonTimer.reset();
        }
        if(lock) ImGui::EndDisabled();

        if(lockButtons) ImGui::EndDisabled();

        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 0, 0, 1));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9, 0, 0, 1));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.7, 0, 0, 1));

        if(ImGui::Button("E-STOP")) EStop::ESTOP();

        ImGui::PopStyleColor(3);

        // If the start button is being pushed, show the little circle thingy
        if(pushed) {
            ImGui::SameLine();
            ImGui::CircleProgressBar("##startlabel", 10, 3, WHITE_COLOR, startTimer.timeSince() / CONFIRM_HOLD_TIME);
        }

        // Display the test number chooser. If there are tests defined in the target
        // json, display a dropdown chooser, otherwise display text saying no tests available
        const std::map<uint8_t, std::string>* tests = TestState::getTestOptions();
        ImGui::Text("Select Test: ");
        ImGui::PushID("testselectcombo");
        if(tests->empty()) {
            ImGui::SameLine();
            ImGui::PushFont(font_italic);
            ImGui::Text("No Available Tests");
            ImGui::PopFont();
        }
        else if(ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.95f),
                ImGui::BeginCombo("##testselect", tests->at(activeTest).c_str())) {
            for(const auto& tn : *tests | std::views::keys) {
                bool selected = tn == activeTest;
                ImGui::PushID(tn);
                if(ImGui::Selectable(tests->at(tn).c_str(), &selected)) activeTest = tn;
                ImGui::PopID();
                if(selected) ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
        ImGui::PopID();

        // Enabling data streaming means that sensor values are streamed back to the host. It is not necessary for
        // a test to be running for this to occur
        ImGui::Text("Enable Data Streaming: ");
        ImGui::SameLine();
        if(lockButtons) ImGui::BeginDisabled();
        dstream = TestState::getDataStreaming();
        if(ImGui::Checkbox("##datastreamingcheckbox", &dstream)) {
            TestState::setDataStreaming(dstream);
            buttonTimer.reset();
        }
        if(lockButtons) ImGui::EndDisabled();

        ImGui::Text("Send Heartbeat Packets: ");
        ImGui::SameLine();
        if(ImGui::Checkbox("##doheartbeats", &doHeartbeats)) {
            if(doHeartbeats) inputHeartbeatRate = 0;
            else {
                TestState::setHeartbeatTime(0);
                buttonTimer.reset();
            }
        }

        // Only show this part if heartbeats are enabled
        if(doHeartbeats) {
            ImGui::Text("Time between heartbeats (seconds): ");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(scale(75));
            ImGui::InputInt("##heartbeatrate", &inputHeartbeatRate);
            if(inputHeartbeatRate < 0) inputHeartbeatRate = 0;
            if(inputHeartbeatRate > 14) inputHeartbeatRate = 14;

            if(lockButtons) ImGui::BeginDisabled();

            int curHeart = TestState::getHeartbeatTime();
            bool restyle = inputHeartbeatRate != curHeart;
            if(restyle) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 1, 0, 1));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0.9, 0, 1));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0.7, 0, 1));
            }

            if(ImGui::Button("Confirm##heartbeatconfirm")) {
                TestState::setHeartbeatTime(inputHeartbeatRate);
                buttonTimer.reset();
            }

            if(restyle) ImGui::PopStyleColor(3);

            if(lockButtons) ImGui::EndDisabled();
        }

        ImGui::Text("Reset sensor time base on start: ");
        ImGui::SameLine();
        if(ImGui::Checkbox("##resettimebox", &resetTimeOnTestStart))
            TestState::setResetTimeOnTestStart(resetTimeOnTestStart);

        if(lockButtons) ImGui::BeginDisabled();

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 0, 0, 1));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9, 0, 0, 1));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.7, 0, 0, 1));

        pushed = false;
        if(ImGui::TimedButton("Hardware Reset", dResetTimer)) {
            pushed = true;
            if(dResetTimer.timeSince() > CONFIRM_HOLD_TIME) {
                TestState::deviceReset();
                buttonTimer.reset();
            }
        }

        if(ImGui::IsItemHovered()) ImGui::SetTooltip("Hold to confirm");
        ImGui::PopStyleColor(3);

        if(lockButtons) ImGui::EndDisabled();

        if(pushed) {
            ImGui::SameLine();
            ImGui::CircleProgressBar("##dresetspinnny", 10, 3, WHITE_COLOR,
                                     dResetTimer.timeSince() / CONFIRM_HOLD_TIME);
        }

        if(!TestState::getInited()) ImGui::EndDisabled();
        ImGui::PopID();
        ImGui::PopID();
    }
} // namespace LRI::RCI
