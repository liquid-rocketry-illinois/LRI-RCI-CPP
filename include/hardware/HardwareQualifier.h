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
    inline const char* deepCopyString(const char* src) {
        size_t slen = strlen(src);
        char* buf = new char[slen + 1];
        memcpy(buf, src, slen);
        buf[slen] = 0;
        return buf;
    }

    struct HardwareQualifier {

        RCP_DeviceClass devclass;
        uint8_t id = 0;
        const char* name;

        HardwareQualifier(RCP_DeviceClass devclass, uint8_t id, const std::string& name) :
            devclass(devclass), id(id), name(deepCopyString(name.c_str())) {}

        HardwareQualifier(RCP_DeviceClass devclass, uint8_t id) : HardwareQualifier(devclass, id, "") {}

        HardwareQualifier(const HardwareQualifier& oq) :
            devclass(oq.devclass), id(oq.id), name(deepCopyString(oq.name)) {}

        HardwareQualifier& operator=(const HardwareQualifier& oq) {
            if(&oq == this) return *this;
            delete[] name;

            devclass = oq.devclass;
            id = oq.id;
            name = deepCopyString(oq.name);

            return *this;
        }

        virtual ~HardwareQualifier() { delete[] name; }

        // Used for ordering
        auto operator<=>(const HardwareQualifier& rhf) const {
            return std::tie(devclass, id) <=> std::tie(rhf.devclass, rhf.id);
        }

        // Helper for packing data as a string. Not for display, use the name field instead
        [[nodiscard]] virtual std::string asString() const {
            return std::format("0x{:2X}-{}-{}", static_cast<uint8_t>(devclass), id, name[0] == 0 ? "<none>" : name);
        }
    };

    struct HardwareChannel : HardwareQualifier {
        ~HardwareChannel() override = default;

        uint8_t channel = 0;

        HardwareChannel(RCP_DeviceClass devclass, uint8_t id, uint8_t channel) :
            HardwareQualifier(devclass, id), channel(channel) {}
        HardwareChannel(const HardwareQualifier& qual, uint8_t channel) : HardwareQualifier(qual), channel(channel) {}

        // Used for ordering
        auto operator<=>(const HardwareChannel& rhf) const {
            return std::tie(devclass, id, channel) <=> std::tie(rhf.devclass, rhf.id, rhf.channel);
        }

        [[nodiscard]] std::string asString() const override {
            return std::format("{}-{}", HardwareQualifier::asString(), channel);
        }
    };
} // namespace LRI::RCI

template<>
struct std::formatter<LRI::RCI::HardwareQualifier> : std::formatter<std::string> {
    auto format(LRI::RCI::HardwareQualifier qual, format_context& ctx) const {
        return format_to(ctx.out(), "0x{:2X}-{}-{}", static_cast<uint8_t>(qual.devclass), qual.id,
                         qual.name[0] == 0 ? "<none>" : qual.name);
    }
};

template<>
struct std::formatter<LRI::RCI::HardwareChannel> : std::formatter<std::string> {
    auto format(LRI::RCI::HardwareChannel qual, format_context& ctx) const {
        return format_to(ctx.out(), "0x{:2X}-{}-{}-{}", static_cast<uint8_t>(qual.devclass), qual.id,
                         qual.name[0] == 0 ? "<none>" : qual.name, qual.channel);
    }
};


#endif // HARDWAREQUALIFIER_H
