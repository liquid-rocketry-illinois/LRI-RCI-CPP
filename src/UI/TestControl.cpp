#include "UI/TestControl.h"

#include "utils.h"

namespace LRI::RCI {
    TestControl* TestControl::instance;

    TestControl* const TestControl::getInstance() {
        if(instance == nullptr) instance = new TestControl();
        return instance;
    }

    TestControl::TestControl() : testState(TEST_STOPPED), testNumber(0) {
        refresh.reset();
        heartbeat.reset();
    }


    void TestControl::render() {
        ImGui::SetNextWindowPos(scale(ImVec2(650, 50)), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(scale(ImVec2(360, 200)), ImGuiCond_FirstUseEver);
        if(ImGui::Begin("Test Control")) {
            ImGui::Text("Test Control");

            bool lockButtons = buttonTimer.timeSince() < buttonDelay;
            if(lockButtons) ImGui::BeginDisabled();

            if(testState != TEST_STOPPED) ImGui::BeginDisabled();
            if(ImGui::Button("Start")) {
                RCP_sendTestUpdate(TEST_START, testNumber);
                buttonTimer.reset();
                testState = TEST_START;
            }
            if(testState != TEST_STOPPED) ImGui::EndDisabled();

            if(testState != TEST_RUNNING && testState != TEST_PAUSED) ImGui::BeginDisabled();
            ImGui::SameLine();
            if(ImGui::Button("End")) {
                RCP_sendTestUpdate(TEST_STOP, testNumber);
                buttonTimer.reset();
                testState = TEST_STOP;
            }
            if(testState != TEST_RUNNING && testState != TEST_PAUSED) ImGui::EndDisabled();

            if(testState != TEST_RUNNING || testState != TEST_PAUSED) ImGui::BeginDisabled();
            ImGui::SameLine();
            if(ImGui::Button(testState == TEST_PAUSE ? "Resume" : "Pause")) {
                RCP_sendTestUpdate(TEST_PAUSE, testNumber);
                buttonTimer.reset();
                testState = testState == TEST_RUNNING ? TEST_PAUSED : TEST_RUNNING;
            }
            if(testState != TEST_RUNNING || testState != TEST_PAUSED) ImGui::EndDisabled();

            if(lockButtons) ImGui::EndDisabled();

            ImGui::SameLine();
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 0, 0, 1));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9, 0, 0, 1));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.7, 0, 0, 1));

            if(ImGui::Button("E-STOP")) RCP_sendEStop();

            ImGui::PopStyleColor(3);

            ImGui::Text("Test Number: ");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(scale(75));
            ImGui::InputInt("##testinput", &testNumber, 1);
            if(testNumber < 0) testNumber = 0;
            if(testNumber > 15) testNumber = 15;

            ImGui::Text("Enable Data Streaming: ");
            ImGui::SameLine();
            if(lockButtons) ImGui::BeginDisabled();
            if(ImGui::Checkbox("##datastreamingcheckbox", &dataStreaming)) {
                buttonTimer.reset();
            }
            if(lockButtons) ImGui::EndDisabled();

            ImGui::Text("Send Heartbeat Packets: ");
            ImGui::SameLine();
            ImGui::Checkbox("##doheartbeats", &doHeartbeats);

            if(doHeartbeats) {
                ImGui::Text("Time between hearbeats (seconds): ");
                ImGui::SameLine();
                ImGui::SetNextItemWidth(scale(75));
                ImGui::InputInt("##heartbeatrate", &heartbeatRate);
                if(heartbeatRate < 0) heartbeatRate = 0;
                if(heartbeatRate > 14) heartbeatRate = 14;

                if(lockButtons) ImGui::BeginDisabled();
                if(ImGui::Button("Confirm##heartbeatconfirm")) {
                    buttonTimer.reset();
                }
                if(lockButtons) ImGui::EndDisabled();
            }
        }

        ImGui::End();
    }

    void TestControl::receiveRCPUpdate(const RCP_TestData data) {
        testNumber = data.selectedTest;
        testState = data.state;
        dataStreaming = data.dataStreaming != 0;
    }
}