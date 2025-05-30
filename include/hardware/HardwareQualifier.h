#ifndef HARDWAREQUALIFIER_H
#define HARDWAREQUALIFIER_H

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
        bool operator<(HardwareQualifier const& rhf) const;

        // Helper for packing data as a string. Not for display, use the name field instead
        [[nodiscard]] std::string asString() const;
    };
} // namespace LRI::RCI

#endif // HARDWAREQUALIFIER_H
