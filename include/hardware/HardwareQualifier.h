#ifndef HARDWAREQUALIFIER_H
#define HARDWAREQUALIFIER_H

#include <format>
#include <string>

#include "RCP_Host/RCP_Host.h"

// Class that represents a device on the target. Each device can be
// fully qualifier using its Device Class and its ID. The name field
// is optional and used for display purposes. It does not have any
// affect on the equality of two HardwareQualifier instances
namespace LRI::RCI {
    struct HardwareQualifier {
        RCP_DeviceClass devclass;
        uint8_t id = 0;
        std::string name;

        // Used for ordering
        bool operator<(HardwareQualifier const& rhf) const {
            return std::tie(devclass, id) < std::tie(rhf.devclass, rhf.id);
        }

        // Helper for packing data as a string. Not for display, use the name field instead
        [[nodiscard]] std::string asString() const {
            return std::format("0x{:2X}-{}-{}", static_cast<uint8_t>(devclass), id, name);
        }
    };
} // namespace LRI::RCI

#endif // HARDWAREQUALIFIER_H
