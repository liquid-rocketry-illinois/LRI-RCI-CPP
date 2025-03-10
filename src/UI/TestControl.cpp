#include "UI/TestControl.h"

#include "utils.h"

#include <iostream>

namespace LRI::RCI {
    TestControl* TestControl::instance;

    TestControl* TestControl::getInstance() {
        if(instance == nullptr) instance = new TestControl();
        return instance;
    }

    TestControl::TestControl() : testState(RCP_TEST_STOPPED), testNumber(0), dataStreaming(false), doHeartbeats(false),
                                 heartbeatRate(0), inputHeartbeatRate(0) {
        heartbeat.reset();
    }


    void TestControl::render() {
        // How heartbeats are handled is a little WIP since if a heartbeat is missed nothing much really happens on the
        // host side. The target should still respond to heartbeats though
        if(doHeartbeats && heartbeatRate != 0 && static_cast<double>(heartbeat.timeSince()) > heartbeatRate * 0.9) {
            RCP_sendHeartbeat();
            heartbeat.reset();
        }

        ImGui::SetNextWindowPos(scale(ImVec2(50, 300)), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(scale(ImVec2(360, 200)), ImGuiCond_FirstUseEver);
        if(ImGui::Begin("Test Control")) {
            ImGui::Text("Test Control");

            bool lockButtons = buttonTimer.timeSince() < BUTTON_DELAY;
            if(lockButtons) ImGui::BeginDisabled();

            // Display controls for starting, stopping, pausing, estopping, and selecting a test
            bool lock2 = testState != RCP_TEST_STOPPED;
            if(lock2) ImGui::BeginDisabled();
            if(ImGui::Button("Start")) {
                RCP_startTest(testNumber);
                buttonTimer.reset();
                testState = RCP_TEST_START;
            }
            if(lock2) ImGui::EndDisabled();

            lock2 = testState != RCP_TEST_RUNNING && testState != RCP_TEST_PAUSED;
            if(lock2) ImGui::BeginDisabled();
            ImGui::SameLine();
            if(ImGui::Button("End")) {
                RCP_changeTestProgress(RCP_TEST_STOP);
                buttonTimer.reset();
                testState = RCP_TEST_STOP;
            }
            if(lock2) ImGui::EndDisabled();

            lock2 = testState != RCP_TEST_RUNNING || testState != RCP_TEST_PAUSED;
            if(lock2) ImGui::BeginDisabled();
            ImGui::SameLine();
            if(ImGui::Button(testState == RCP_TEST_PAUSE ? "Resume" : "Pause")) {
                RCP_changeTestProgress(RCP_TEST_PAUSE);
                buttonTimer.reset();
                testState = testState == RCP_TEST_RUNNING ? RCP_TEST_PAUSED : RCP_TEST_RUNNING;
            }
            if(lock2) ImGui::EndDisabled();

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

            // Enabling data streaming means that sensor values are streamed back to the host. It is not necessary for
            // a test to be running for this to occur
            ImGui::Text("Enable Data Streaming: ");
            ImGui::SameLine();
            if(lockButtons) ImGui::BeginDisabled();
            if(ImGui::Checkbox("##datastreamingcheckbox", &dataStreaming)) {
                RCP_setDataStreaming(dataStreaming);
                buttonTimer.reset();
            }
            if(lockButtons) ImGui::EndDisabled();

            ImGui::Text("Send Heartbeat Packets: ");
            ImGui::SameLine();
            if(ImGui::Checkbox("##doheartbeats", &doHeartbeats)) {
                if(doHeartbeats) heartbeatRate = 0;
                else {
                    RCP_setHeartbeatTime(0);
                    buttonTimer.reset();
                }
            }

            if(doHeartbeats) {
                ImGui::Text("Time between hearbeats (seconds): ");
                ImGui::SameLine();
                ImGui::SetNextItemWidth(scale(75));
                ImGui::InputInt("##heartbeatrate", &inputHeartbeatRate);
                if(inputHeartbeatRate < 0) inputHeartbeatRate = 0;
                if(inputHeartbeatRate > 14) inputHeartbeatRate = 14;

                if(lockButtons) ImGui::BeginDisabled();

                bool restyle = inputHeartbeatRate != heartbeatRate;
                if(restyle) {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 1, 0, 1));
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0.9, 0, 1));
                    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0.7, 0, 1));
                }

                if(ImGui::Button("Confirm##heartbeatconfirm")) {
                    heartbeatRate = inputHeartbeatRate;
                    RCP_setHeartbeatTime(heartbeatRate);
                    buttonTimer.reset();
                }

                if(restyle) ImGui::PopStyleColor(3);

                if(lockButtons) ImGui::EndDisabled();
            }
        }

        ImGui::End();
    }

    void TestControl::receiveRCPUpdate(const RCP_TestData& data) {
        heartbeatRate = data.heartbeatTime;
        inputHeartbeatRate = data.heartbeatTime;
        testState = data.state;
        dataStreaming = data.dataStreaming != 0;
    }

    void TestControl::reset() {
        testState = RCP_TEST_STOPPED;
        testNumber = 0;
        dataStreaming = false;
        doHeartbeats = false;
        inputHeartbeatRate = 0;
    }
}
