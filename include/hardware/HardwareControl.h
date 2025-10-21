#ifndef LRI_CONTROL_PANEL_HARDWARECONTROL_H
#define LRI_CONTROL_PANEL_HARDWARECONTROL_H

#include <chrono>
#include <set>

#include "HardwareQualifier.h"
#include "interfaces/RCP_Interface.h"

namespace LRI::RCI::HWCTRL {
    extern int POLLS_PER_UPDATE;

    enum class ErrorType {
        RCP_STREAM,
        GENERAL_RCP,
        HWNE_HOST,
        HWNE_TARGET,
    };

    struct Error {
        std::chrono::system_clock::time_point time;
        ErrorType type;
        std::string what;

        Error(ErrorType type, const std::string& what) :
            time(std::chrono::system_clock::now()), type(type), what(what) {}

        Error(ErrorType type, const HardwareQualifier& qual) : Error(type, "") {
            if(type == ErrorType::HWNE_HOST)
                what = "Hardware Qualifier requested that was not present in the configuration: " + qual.asString();
            else
                what = "Hardware Qualifier received from target that was not present in the configuration: " +
                    qual.asString();
        }
    };

    void start(RCP_Interface* interf);
    void update();
    void pause();
    void end();

    void setHardwareConfig(const std::set<HardwareQualifier>& quals);

    void addError(const Error& e);
    const std::vector<Error>& getErrors();
    bool UIHasNewErrors();
} // namespace LRI::RCI::HWCTRL

#endif // LRI_CONTROL_PANEL_HARDWARECONTROL_H
