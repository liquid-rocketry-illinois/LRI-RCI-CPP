#include "UI/TestStateViewer.h"

#include "hardware/EStop.h"
#include "hardware/TestState.h"
#include "utils.h"

namespace LRI::RCI {
    int TestStateViewer::CLASSID = 0;

    TestStateViewer::TestStateViewer() : classid(CLASSID++), inputHeartbeatRate(0), inputTestNum(0), dstream(false),
                                         doHeartbeats(false) {
    }


    void TestStateViewer::render() {
        ImGui::PushID("TestStateViewer");
        ImGui::PushID(classid);

        ImGui::Text("Test Control");

        bool lockButtons = buttonTimer.timeSince() < BUTTON_DELAY;
        if(lockButtons) ImGui::BeginDisabled();

        // Display controls for starting, stopping, pausing, estopping, and selecting a test
        RCP_TestRunningState_t state = TestState::getInstance()->getState();

        bool lock = state != RCP_TEST_STOPPED;
        if(lock) ImGui::BeginDisabled();
        if(ImGui::Button("Start")) {
            TestState::getInstance()->startTest(inputTestNum);
            buttonTimer.reset();
        }
        if(lock) ImGui::EndDisabled();

        lock = state != RCP_TEST_RUNNING && state != RCP_TEST_PAUSED;
        if(lock) ImGui::BeginDisabled();
        ImGui::SameLine();
        if(ImGui::Button("End")) {
            TestState::getInstance()->stopTest();
            buttonTimer.reset();
        }
        if(lock) ImGui::EndDisabled();

        lock = state != RCP_TEST_RUNNING || state != RCP_TEST_PAUSED;
        if(lock) ImGui::BeginDisabled();
        ImGui::SameLine();
        if(ImGui::Button(state == RCP_TEST_PAUSE ? "Resume" : "Pause")) {
            TestState::getInstance()->pause();
            buttonTimer.reset();
        }
        if(lock) ImGui::EndDisabled();

        if(lockButtons) ImGui::EndDisabled();

        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 0, 0, 1));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9, 0, 0, 1));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.7, 0, 0, 1));

        if(ImGui::Button("E-STOP")) EStop::getInstance()->ESTOP();

        ImGui::PopStyleColor(3);

        ImGui::Text("Test Number: ");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(scale(75));
        ImGui::InputInt("##testinput", &inputTestNum, 0);
        if(inputTestNum < 0) inputTestNum = 0;
        if(inputTestNum > 15) inputTestNum = 15;
        int curTest = TestState::getInstance()->getTestNum();
        if(inputTestNum != curTest) {
            ImGui::SameLine();
            ImGui::PushFont(font_italic);
            ImGui::Text("Current: %d", curTest);
            ImGui::PopFont();
        }

        // Enabling data streaming means that sensor values are streamed back to the host. It is not necessary for
        // a test to be running for this to occur
        ImGui::Text("Enable Data Streaming: ");
        ImGui::SameLine();
        if(lockButtons) ImGui::BeginDisabled();
        dstream = TestState::getInstance()->getDataStreaming();
        if(ImGui::Checkbox("##datastreamingcheckbox", &dstream)) {
            TestState::getInstance()->setDataStreaming(dstream);
            buttonTimer.reset();
        }
        if(lockButtons) ImGui::EndDisabled();

        ImGui::Text("Send Heartbeat Packets: ");
        ImGui::SameLine();
        if(ImGui::Checkbox("##doheartbeats", &doHeartbeats)) {
            if(doHeartbeats) inputHeartbeatRate = 0;
            else {
                TestState::getInstance()->setHeartbeatTime(0);
                buttonTimer.reset();
            }
        }

        if(doHeartbeats) {
            ImGui::Text("Time between heartbeats (seconds): ");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(scale(75));
            ImGui::InputInt("##heartbeatrate", &inputHeartbeatRate);
            if(inputHeartbeatRate < 0) inputHeartbeatRate = 0;
            if(inputHeartbeatRate > 14) inputHeartbeatRate = 14;

            if(lockButtons) ImGui::BeginDisabled();

            int curHeart = TestState::getInstance()->getHeartbeatTime();
            bool restyle = inputHeartbeatRate != curHeart;
            if(restyle) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 1, 0, 1));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0.9, 0, 1));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0.7, 0, 1));
            }

            if(ImGui::Button("Confirm##heartbeatconfirm")) {
                TestState::getInstance()->setHeartbeatTime(inputHeartbeatRate);
                buttonTimer.reset();
            }

            if(restyle) ImGui::PopStyleColor(3);

            if(lockButtons) ImGui::EndDisabled();
        }

        ImGui::PopID();
        ImGui::PopID();
    }

    void TestStateViewer::reset() {
        inputHeartbeatRate = 0;
    }
}
