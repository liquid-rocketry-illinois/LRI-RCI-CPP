#ifndef HARDWAREQUALIFIER_H
#define HARDWAREQUALIFIER_H

#include <string>
#include "RCP_Host/RCP_Host.h"

namespace LRI::RCI {
    struct HardwareQualifier {
        RCP_DeviceClass devclass;
        uint8_t id = 0;
        std::string name;

        // Used for ordering
        bool operator<(HardwareQualifier const& rhf) const;

        // Helper for packing data as a string
        [[nodiscard]] std::string asString() const;
    };
}

#endif //HARDWAREQUALIFIER_H
