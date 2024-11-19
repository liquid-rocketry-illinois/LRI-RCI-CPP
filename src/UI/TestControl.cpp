#include "UI/TestControl.h"

#include "utils.h"

namespace LRI::RCI {
    TestControl* TestControl::instance;

    TestControl* const TestControl::getInstance() {
        if(instance == nullptr) instance = new TestControl();
        return instance;
    }

    TestControl::TestControl() : testState(TEST_PAUSE), testNumber(0) {
        refresh.reset();
        heartbeat.reset();
    }


    void TestControl::render() {
        ImGui::SetNextWindowPos(scale(ImVec2(650, 50), scaling_factor), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(scale(ImVec2(200, 200), scaling_factor), ImGuiCond_FirstUseEver);
        if(ImGui::Begin("Test Control")) {
            ImGui::Text("Test Control");

            if(testState != TEST_STOPPED) ImGui::BeginDisabled();
            if(ImGui::Button("Start")) {
                RCP_sendTestUpdate(TEST_START, testNumber);
                testState = TEST_START;
            }
            if(testState != TEST_STOPPED) ImGui::EndDisabled();

            if(testState != TEST_RUNNING) ImGui::BeginDisabled();
            ImGui::SameLine();
            if(ImGui::Button("End")) {
                RCP_sendTestUpdate(TEST_STOP, testNumber);
                testState = TEST_STOP;
            }
            if(testState != TEST_RUNNING) ImGui::EndDisabled();

            if(testState != TEST_RUNNING || testState != TEST_PAUSED) ImGui::BeginDisabled();
            ImGui::SameLine();
            if(ImGui::Button(testState == TEST_PAUSE ? "Resume" : "Pause")) {
                RCP_sendTestUpdate(TEST_PAUSE, testNumber);
                testState = testState == TEST_RUNNING ? TEST_PAUSED : TEST_RUNNING;
            }
            if(testState != TEST_RUNNING || testState != TEST_PAUSED) ImGui::EndDisabled();

            ImGui::SameLine();
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 0, 0, 1));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9, 0, 0, 1));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.7, 0, 0, 1));

            if(ImGui::Button("E-STOP")) RCP_sendEStop();

            ImGui::PopStyleColor(3);

            ImGui::Text("Test Number: ");
            ImGui::SameLine();
            ImGui::InputInt("##testinput", &testNumber, 1);
            if(testNumber < 0) testNumber = 0;
            if(testNumber > 15) testNumber = 15;

            ImGui::Text("Enable Data Streaming: ");
            ImGui::SameLine();
            ImGui::Checkbox("##datastreamingcheckbox", &dataStreaming);

            ImGui::Text("Send Heartbeat Packets: ");
            ImGui::Checkbox("##doheartbeats", &doHeartbeats);

            if(doHeartbeats) {
                ImGui::SameLine();
                ImGui::Text("Time between hearbeats (seconds): ");
                ImGui::SameLine();
                ImGui::InputInt("##heartbeatrate", &heartbeatRate);
                if(heartbeatRate < 0) heartbeatRate = 0;
                if(heartbeatRate > 14) heartbeatRate = 14;
                ImGui::SameLine();
                if(ImGui::Button("Confirm##heartbeatconfirm")) {

                }
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