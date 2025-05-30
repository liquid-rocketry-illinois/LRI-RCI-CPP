#include "hardware/HardwareQualifier.h"

namespace LRI::RCI {
    bool HardwareQualifier::operator<(HardwareQualifier const& rhf) const {
        if(devclass == rhf.devclass) return id < rhf.id;
        return devclass < rhf.devclass;
    }

    // Used in imgui IDs, not necessarily meant for display. In the format of DEVCLASS-ID-NAME
    std::string HardwareQualifier::asString() const {
        return std::to_string(devclass) + "-" + std::to_string(id) + "-" + name;
    }
} // namespace LRI::RCI
