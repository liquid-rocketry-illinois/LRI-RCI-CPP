#ifndef LRI_CONTROL_PANEL_HARDWARECONTROL_H
#define LRI_CONTROL_PANEL_HARDWARECONTROL_H

#include <chrono>
#include <map>
#include <set>
#include <vector>

// #include "EventLog.h"
#include "HardwareQualifier.h"

#include "interfaces/RCP_Interface.h"

namespace LRI::RCI {
    namespace HWCTRL {
        extern int POLLS_PER_UPDATE;
        extern int PACKETS_POLLED_IN_LAST_FRAME;

        enum class ErrorType { RCP_STREAM, GENERAL_RCP, HWNE_HOST, HWNE_TARGET, HW_INVALID, MISALIGNED_ELOG };

        struct Error {
            std::chrono::system_clock::time_point time;
            ErrorType type;
            std::string what;

            Error(ErrorType type, std::string what) :
                time(std::chrono::system_clock::now()), type(type), what(std::move(what)) {}

            Error(ErrorType type, const HardwareQualifier& qual) : Error(type, "") {
                if(type == ErrorType::HWNE_HOST)
                    what = "Hardware Qualifier requested that was not present in the configuration: " + qual.asString();
                else
                    what = "Hardware Qualifier received from target that was not present in the configuration: " +
                        qual.asString();
            }

            Error(ErrorType type, const HardwareChannel& ch) : Error(type, "") {
                if(type == ErrorType::HWNE_HOST)
                    what = "Hardware Channel requested that was not present in the configuration: " + ch.asString();
            }
        };

        const std::vector<Error>& getErrors();

        void start(RCP_Interface* interf, std::set<HardwareQualifier> quals);
        void update();
        void pause();
        void end();
        bool isOpen();

        void requestRefresh(const HardwareQualifier& qual);
    } // namespace HWCTRL

    namespace AngledActuator {
        void setActuatorPos(const HardwareQualifier& qual, float degrees);
    }

    namespace Motor {
        void setSpeed(const HardwareQualifier& qual, float rpm);
    }

    namespace Prompt {
        // Getters for class members
        [[nodiscard]] bool isActivePrompt();
        [[nodiscard]] const std::string& get_prompt();
        [[nodiscard]] RCP_PromptDataType getType();

        // Submit the latest prompt data
        RCP_Error submitPrompt(float val);
        RCP_Error submitPrompt(bool val);
    } // namespace Prompt

    namespace Stepper {
        void setState(const HardwareQualifier& qual, RCP_StepperControlMode controlMode, float value);
    }

    namespace Log {
        void clearDisplayString();

        // Return stream for display
        [[nodiscard]] const std::string& getDisplayString();
    } // namespace Log

    namespace TestState {
        [[nodiscard]] bool getInited();

        void setTests(const std::map<uint8_t, std::string>& testlist);
        [[nodiscard]] const std::map<uint8_t, std::string>* getTestOptions();

        // Getters for class members
        [[nodiscard]] uint8_t getActiveTest();
        [[nodiscard]] RCP_TestRunningState getState();
        [[nodiscard]] uint8_t getHeartbeatTime();
        [[nodiscard]] bool getDataStreaming();

        // Methods to send requests to target
        void startTest(uint8_t number);
        void stopTest();
        void pause();
        void setHeartbeatTime(uint8_t time);
        void setDataStreaming(bool stream);
        void setResetTimeOnTestStart(bool reset);
        void deviceReset();
        void ESTOP();

        // To be called from main. Handles heartbeats
        void update();

        // Clear inited
        void reset();

        // Receive updates from rcp
        RCP_Error receiveRCPUpdate(RCP_TestData testState);
    } // namespace TestState
} // namespace LRI::RCI
#endif // LRI_CONTROL_PANEL_HARDWARECONTROL_H
