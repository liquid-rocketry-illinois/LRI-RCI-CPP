#include "hardware/HardwareControl.h"

#include <ranges>

#include "hardware/AngledActuator.h"
#include "hardware/BoolSensor.h"
#include "hardware/Prompt.h"
#include "hardware/RawData.h"
#include "hardware/Sensors.h"
#include "hardware/SimpleActuators.h"
#include "hardware/Steppers.h"
#include "hardware/TestState.h"

namespace LRI::RCI::HWCTRL {
    size_t sendData(const void* data, size_t length);
    size_t readData(void* data, size_t bufferSize);
    int processTestUpdate(RCP_TestData data);
    int processBoolData(RCP_BoolData data);
    int processSimpleActuatorData(RCP_SimpleActuatorData data);
    int processPromptInput(RCP_PromptInputRequest request);
    int processSerialData(RCP_CustomData data);
    int processOneFloat(RCP_OneFloat data);
    int processTwoFloat(RCP_TwoFloat data);
    int processThreeFloat(RCP_ThreeFloat data);
    int processFourFloat(RCP_FourFloat data);

    static RCP_LibInitData callbacks = {
        .sendData = sendData,
        .readData = readData,
        .processTestUpdate = processTestUpdate,
        .processBoolData = processBoolData,
        .processSimpleActuatorData = processSimpleActuatorData,
        .processPromptInput = processPromptInput,
        .processSerialData = processSerialData,
        .processOneFloat = processOneFloat,
        .processTwoFloat = processTwoFloat,
        .processThreeFloat = processThreeFloat,
        .processFourFloat = processFourFloat,
    };

    static RCP_Interface* interf;
    static bool doPoll = true;
    static bool hasStarted = false;

    int POLLS_PER_UPDATE = 25;

    void start([[maybe_unused]] RCP_Interface* _interf) {
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
        TestState::getInstance()->update(); // Heartbeats
        Sensors::update(); // Serialization threads

        if(!doPoll) return;
        for(int i = 0; i < POLLS_PER_UPDATE; i++) {
            if(interf->pktAvailable()) {
                int rc = RCP_poll();
                if(rc == 0) return;

                if(rc == -1) addError({ErrorType::GENERAL_RCP, "RCP Not Initialized correctly"});
                else if(rc == -2) addError({ErrorType::RCP_STREAM, "RCP target stream has become corrupted"});
                else if(rc != 1) addError({ErrorType::GENERAL_RCP, "Unknown RCP error encountered"});
                pause();
            }
        }
    }

    void pause() { doPoll = !doPoll; }

    static void resetHardware() {
        // AngledActuators MUST be reset before Sensors
        AngledActuators::reset();
        BoolSensors::reset();
        Prompt::reset();
        RawData::reset();
        Sensors::reset();
        SimpleActuators::reset();
        Steppers::reset();
        TestState::getInstance()->reset();
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
        RCP_DEVCLASS_AM_TEMPERATURE,
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

    bool UIHasNewErrors() {
        if(newErrors) {
            newErrors = false;
            return true;
        }

        return false;
    }


    // This section contains all the callbacks needed for RCP. They simply forward data to the respective singletons
    size_t sendData(const void* data, size_t length) { return interf->sendData(data, length); }

    size_t readData(void* data, size_t bufferSize) { return interf->readData(data, bufferSize); }

    int processTestUpdate(const RCP_TestData data) {
        TestState::getInstance()->receiveRCPUpdate(data);
        return 0;
    }

    int processBoolData(RCP_BoolData data) {
        return BoolSensors::receiveRCPUpdate({RCP_DEVCLASS_BOOL_SENSOR, data.ID, ""}, data.data);
    }

    int processSimpleActuatorData(const RCP_SimpleActuatorData data) {
        return SimpleActuators::receiveRCPUpdate({RCP_DEVCLASS_SIMPLE_ACTUATOR, data.ID, ""},
                                                 data.state == RCP_SIMPLE_ACTUATOR_ON);
    }

    int processPromptInput(RCP_PromptInputRequest request) {
        Prompt::receiveRCPUpdate(request);
        return 0;
    }

    int processSerialData(const RCP_CustomData data) {
        RawData::receiveRCPUpdate(data);
        return 0;
    }

    int processOneFloat(RCP_OneFloat data) { return Sensors::receiveRCPUpdate(data); }

    int processTwoFloat(RCP_TwoFloat data) {
        if(data.devclass == RCP_DEVCLASS_STEPPER)
            return Steppers::receiveRCPUpdate({RCP_DEVCLASS_STEPPER, data.ID, ""}, data.data[0], data.data[1]);
        return Sensors::receiveRCPUpdate(data);
    }

    int processThreeFloat(RCP_ThreeFloat data) { return Sensors::receiveRCPUpdate(data); }

    int processFourFloat(RCP_FourFloat data) { return Sensors::receiveRCPUpdate(data); }
} // namespace LRI::RCI::HWCTRL
