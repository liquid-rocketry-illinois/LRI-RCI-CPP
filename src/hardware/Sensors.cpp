#include "hardware/Sensors.h"

#include <chrono>s

#include "hardware/HardwareControl.h"

namespace LRI::RCI::Sensors {
    // Holds the data vectors mapped to their qualifiers
    static std::map<HardwareQualifier, float[4]> sensors;

    RCP_Error receiveRCPUpdate1(const RCP_1F& data) {
        HardwareQualifier qual = {data.devclass, data.ID};
        if(!sensors.contains(qual)) {
            HWCTRL::addError({HWCTRL::ErrorType::HWNE_TARGET, qual});
            return RCP_ERR_INVALID_DEVCLASS;
        }

        sensors[qual][0] = data.data;
        EventLog::getGlobalLog().add1F(data);
        return RCP_ERR_SUCCESS;
    }

    RCP_Error receiveRCPUpdate2(const RCP_2F& data) {
        HardwareQualifier qual = {data.devclass, data.ID};
        if(!sensors.contains(qual)) {
            HWCTRL::addError({HWCTRL::ErrorType::HWNE_TARGET, qual});
            return RCP_ERR_INVALID_DEVCLASS;
        }

        for(int i = 0; i < 2; i++) sensors[qual][i] = data.data[i];
        EventLog::getGlobalLog().add2F(data);
        return RCP_ERR_SUCCESS;
    }

    RCP_Error receiveRCPUpdate3(RCP_3F data) {
        HardwareQualifier qual = {data.devclass, data.ID};
        if(!sensors.contains(qual)) {
            HWCTRL::addError({HWCTRL::ErrorType::HWNE_TARGET, qual});
            return RCP_ERR_INVALID_DEVCLASS;
        }

        for(int i = 0; i < 3; i++) sensors[qual][i] = data.data[i];
        EventLog::getGlobalLog().add3F(data);
        return RCP_ERR_SUCCESS;
    }

    RCP_Error receiveRCPUpdate4(RCP_4F data) {
        HardwareQualifier qual = {data.devclass, data.ID};
        if(!sensors.contains(qual)) {
            HWCTRL::addError({HWCTRL::ErrorType::HWNE_TARGET, qual});
            return RCP_ERR_INVALID_DEVCLASS;
        }

        for(int i = 0; i < 4; i++) sensors[qual][i] = data.data[i];
        EventLog::getGlobalLog().add4F(data);
        return RCP_ERR_SUCCESS;
    }

    void setHardwareConfig(const std::set<HardwareQualifier>& quals) {
        reset();
        for(const auto& qual : quals) sensors[qual][0] = 0;
    }

    const float* getLatestState(const HardwareChannel& ch) {
        if(!sensors.contains(ch)) {
            HWCTRL::addError({HWCTRL::ErrorType::HWNE_HOST, ch});
            return nullptr;
        }

        return &sensors[ch][ch.channel];
    }


    FloatData getFullLog(const HardwareChannel& ch) {
        if(!sensors.contains(ch)) {
            HWCTRL::addError({HWCTRL::ErrorType::HWNE_HOST, ch});
            return {};
        }

        return {&EventLog::getGlobalLog().getSensorTimestamps().at(ch), &EventLog::getGlobalLog().getFloats().at(ch)};
    }

    void tare(const HardwareChannel& ch) {
        if(!sensors.contains(ch)) {
            HWCTRL::addError({HWCTRL::ErrorType::HWNE_HOST, ch});
            return;
        }

        auto [times, data] = getFullLog(ch);
        if(times->size() != data->size()) {
            HWCTRL::addError({HWCTRL::ErrorType::MISALIGNED_ELOG, "Time and data vectors have different elements!"});
            return;
        }

        if(times->empty()) return;

        double total = 0;
        size_t numElems = 0;
        std::chrono::system_clock::time_point prevtime = times->at(times->size() - 1).systime - std::chrono::seconds(1);

        for(size_t i = times->size(); i > 0 ; i--) {
            if(times->at(i - 1).systime < prevtime) break;
            numElems++;
            total -= data->at(i - 1);
        }

        auto fave = static_cast<float>(total / static_cast<double>(numElems));
        RCP_requestTareConfiguration(ch.devclass, ch.id, ch.channel, fave);
    }

    void reset() {
        sensors.clear();
    }
} // namespace LRI::RCI::Sensors
