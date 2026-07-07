#ifndef HARDWAREQUALIFIER_H
#define HARDWAREQUALIFIER_H

#include <format>
#include <string>
#include <string_view>
#include <utility>
using namespace std::string_view_literals;
#include "RCP_Host/RCP_Host.h"

/*
Class that represents a device on the target. Each device can be
fully qualifier using its Device Class and its ID. The name field
is optional and used for display purposes. It does not have any
effect on the equality of two HardwareQualifier instances

The return values from each device can be fully qualified with the
hardware qualifier, plus the channel index.

The HardwareChannel should be used in most cases. HardwareQualifier
exists only for operations that do not include channel indices, but
HardwareChannel can still be passed as a parameter to these
operations. The channel is ignored.
*/
namespace LRI::RCI {
    struct HardwareQualifier {
        virtual ~HardwareQualifier() = default;

        RCP_DeviceClass devclass;
        uint8_t id = 0;
        std::string name;

        HardwareQualifier(RCP_DeviceClass devclass, uint8_t id, std::string_view name) :
            devclass(devclass), id(id), name(name) {}
        HardwareQualifier(RCP_DeviceClass devclass, uint8_t id) : HardwareQualifier(devclass, id, "") {}

        // Used for ordering
        auto operator<=>(const HardwareQualifier& rhf) const {
            return std::tie(devclass, id) <=> std::tie(rhf.devclass, rhf.id);
        }

        // Helper for packing data as a string. Not for display, use the name field instead
        [[nodiscard]] virtual std::string asString() const {
            return std::format("0x{:2X}-{}-{}", static_cast<uint8_t>(devclass), id, name);
        }
    };

    struct HardwareChannel : HardwareQualifier {
        ~HardwareChannel() override = default;

        uint8_t channel = 0;

        HardwareChannel(RCP_DeviceClass devclass, uint8_t id, uint8_t channel) :
            HardwareQualifier(devclass, id), channel(channel) {}
        HardwareChannel(const HardwareQualifier& qual, uint8_t channel = 0) :
            HardwareQualifier(qual), channel(channel) {}

        // Used for ordering
        auto operator<=>(const HardwareChannel& rhf) const {
            return std::tie(devclass, id, channel) <=> std::tie(rhf.devclass, rhf.id, rhf.channel);
        }

        [[nodiscard]] std::string asString() const override {
            return std::format("{}-{}", HardwareQualifier::asString(), channel);
        }
    };

    namespace TestStateChannels {
        enum Channels : uint8_t {
            T_INITED = 0,
            T_DSTREAM = 1,
            T_HEARTBEAT_TIME = 2,
            T_TEST_RUN_STATE = 3,
            T_RUNNING_TEST = 4,
            T_TEST_PROGRESS = 5,

            H_HEARTBEAT = 0,
            H_DSTREAM = 1,
            H_HEARTBEAT_TIME = 2,
            H_TEST_RUN_STATE = 3,
            H_TEST_ID = 4,
            H_TIME_RESET = 5,
            H_DEV_RESET = 6
        };
    }
} // namespace LRI::RCI

#endif // HARDWAREQUALIFIER_H
