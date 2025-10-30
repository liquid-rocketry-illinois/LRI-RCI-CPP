#include "hardware/HardwareQualifier.h"

#include <format>

namespace LRI::RCI {
    bool HardwareQualifier::operator<(HardwareQualifier const& rhf) const {
        if(devclass == rhf.devclass) return id < rhf.id;
        return devclass < rhf.devclass;
    }

    // Used in imgui IDs, not necessarily meant for display. In the format of DEVCLASS-ID-NAME
    std::string HardwareQualifier::asString() const {
        return std::format("0x{:2X}-{}-{}", static_cast<uint8_t>(devclass), id, name);
    }
} // namespace LRI::RCI
