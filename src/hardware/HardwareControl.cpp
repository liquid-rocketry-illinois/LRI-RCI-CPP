#include "hardware/HardwareControl.h"

#include <ranges>

#include "hardware/AngledActuator.h"
#include "hardware/BoolSensor.h"
#include "hardware/Prompt.h"
#include "hardware/Sensors.h"
#include "hardware/SimpleActuators.h"
#include "hardware/Steppers.h"
#include "hardware/TargetLog.h"
#include "hardware/TestState.h"

namespace LRI::RCI::HWCTRL {
    size_t sendData(const void* data, size_t length);
    size_t readData(void* data, size_t bufferSize);

    RCP_Error route1F(RCP_1F data);
    RCP_Error route2F(RCP_2F data);

    static RCP_LibInitData callbacks = {
        .sendData = sendData,
        .readData = readData,
        .processTestUpdate = TestState::receiveRCPUpdate,
        .processBoolData = BoolSensors::receiveRCPUpdate,
        .processSimpleActuatorData = SimpleActuators::receiveRCPUpdate,
        .processPromptInput = Prompt::receiveRCPUpdate,
        .processTargetLog = TargetLog::receiveRCPUpdate,
        .processOneFloat = route1F,
        .processTwoFloat = route2F,
        .processThreeFloat = Sensors::receiveRCPUpdate3,
        .processFourFloat = Sensors::receiveRCPUpdate4,
    };

    static RCP_Interface* interf;
    static bool doPoll = true;
    static bool hasStarted = false;

    int POLLS_PER_UPDATE = 25;

    void start(RCP_Interface* _interf) {
        if(interf) {
            return;
        }

        interf = _interf;
        RCP_init(callbacks);
        RCP_setChannel(RCP_CH_ZERO);
        hasStarted = true;
    }

    void update() {
        if(!hasStarted) return;
        BoolSensors::update(); // Periodic updates
        TestState::update(); // Heartbeats

        if(!doPoll) return;
        for(int i = 0; i < POLLS_PER_UPDATE; i++) {
            if(interf->pktAvailable()) {
                RCP_Error rc = RCP_poll();
                if(rc == RCP_ERR_SUCCESS) continue;

                ErrorType type = ErrorType::GENERAL_RCP;
                if(rc == RCP_ERR_IO_RCV || rc == RCP_ERR_IO_SEND) type = ErrorType::RCP_STREAM;
                addError({type, RCP_errstr(rc)});
                pause();
            }
        }
    }

    void pause() { doPoll = !doPoll; }

    static void resetHardware() {
        // AngledActuators MUST be reset before Sensors
        AngledActuators::reset();
        BoolSensors::reset();
        TargetLog::clearDisplayString();
        Sensors::reset();
        SimpleActuators::reset();
        Steppers::reset();
        TestState::reset();
    }

    void end() {
        hasStarted = false;
        RCP_shutdown();
        delete interf;
        interf = nullptr;
        resetHardware();
    }

    const std::set SENSOR_CLASSES = {
        RCP_DEVCLASS_AM_PRESSURE,
        RCP_DEVCLASS_TEMPERATURE,
        RCP_DEVCLASS_PRESSURE_TRANSDUCER,
        RCP_DEVCLASS_RELATIVE_HYGROMETER,
        RCP_DEVCLASS_LOAD_CELL,
        RCP_DEVCLASS_BOOL_SENSOR,

        RCP_DEVCLASS_POWERMON,

        RCP_DEVCLASS_ACCELEROMETER,
        RCP_DEVCLASS_GYROSCOPE,
        RCP_DEVCLASS_MAGNETOMETER,

        RCP_DEVCLASS_GPS,
    };

    void setHardwareConfig([[maybe_unused]] const std::set<HardwareQualifier>& quals) {
        std::set<RCP_DeviceClass> processedClasses;

        for(const auto& qual : quals) {
            if(processedClasses.contains(qual.devclass)) continue;
            processedClasses.insert(qual.devclass);
            auto filtered = quals |
                std::views::filter([&qual](const HardwareQualifier& qual2) { return qual.devclass == qual2.devclass; });
            std::set asSet(filtered.cbegin(), filtered.cend());

            // Handle all the sensors in one so we dont need a really long switch
            if(SENSOR_CLASSES.contains(qual.devclass)) Sensors::setHardwareConfig(asSet);

            else switch(qual.devclass) {
                case RCP_DEVCLASS_SIMPLE_ACTUATOR:
                    SimpleActuators::setHardwareConfig(asSet);
                    break;

                case RCP_DEVCLASS_STEPPER:
                    Steppers::setHardwareConfig(asSet);
                    break;

                case RCP_DEVCLASS_ANGLED_ACTUATOR:
                    AngledActuators::setHardwareConfig(asSet);
                    break;

                default:
                    break;
                }
        }
    }

    static std::vector<Error> errors;
    static bool newErrors = false;

    void addError(const Error& e) {
        errors.push_back(e);
        newErrors = true;
    }

    const std::vector<Error>& getErrors() { return errors; }

    // Could be used for things like toast notifs
    bool UIHasNewErrors() {
        if(newErrors) {
            newErrors = false;
            return true;
        }

        return false;
    }

    // This section contains all the callbacks needed for RCP. They simply forward data to the respective singletons
    size_t sendData(const void* data, size_t length) {
        // Call the interface first, to queue up the data as soon as possible
        size_t ret = interf->sendData(data, length);
        EventLog::getGlobalLog().addSent(data, length);
        return ret;
    }

    size_t readData(void* data, size_t bufferSize) {
        size_t ret = interf->readData(data, bufferSize);
        EventLog::getGlobalLog().addReceived(data, bufferSize);
        return ret;
    }

    RCP_Error route1F(RCP_1F data) {
        if(data.devclass == RCP_DEVCLASS_ANGLED_ACTUATOR) return AngledActuators::receiveRCPUpdate(data);
        return Sensors::receiveRCPUpdate1(data);
    }

    RCP_Error route2F(RCP_2F data) {
        if(data.devclass == RCP_DEVCLASS_STEPPER) return Steppers::receiveRCPUpdate(data);
        return Sensors::receiveRCPUpdate2(data);
    }
} // namespace LRI::RCI::HWCTRL
