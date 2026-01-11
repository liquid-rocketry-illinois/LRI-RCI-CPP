#ifndef HARDWAREQUALIFIER_H
#define HARDWAREQUALIFIER_H

#include <format>
#include <string>

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

        HardwareQualifier(RCP_DeviceClass devclass, uint8_t id, std::string name) :
            devclass(devclass), id(id), name(std::move(name)) {}
        HardwareQualifier(RCP_DeviceClass devclass, uint8_t id) : HardwareQualifier(devclass, id, "") {}

        // Used for ordering
        bool operator<(HardwareQualifier const& rhf) const {
            return std::tie(devclass, id) < std::tie(rhf.devclass, rhf.id);
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
        HardwareChannel(const HardwareQualifier& qual, uint8_t channel) : HardwareQualifier(qual), channel(channel) {}

        bool operator<(HardwareChannel const& rhf) const {
            return std::tie(devclass, id, channel) < std::tie(rhf.devclass, rhf.id, rhf.channel);
        }

        [[nodiscard]] std::string asString() const override {
            return std::format("{}-{}", HardwareQualifier::asString(), channel);
        }
    };
} // namespace LRI::RCI

#endif // HARDWAREQUALIFIER_H
