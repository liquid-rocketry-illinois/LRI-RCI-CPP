#ifndef RAWDATA_H
#define RAWDATA_H

#include <vector>

#include "RCP_Host/RCP_Host.h"

namespace LRI::RCI {
    class RawData {
        std::vector<uint8_t> raw;

        RawData() = default;
        ~RawData() = default;

    public:
        static RawData* getInstance();

        void receiveRCPUpdate(const RCP_CustomData& data);

        void reset();

        [[nodiscard]] const std::vector<uint8_t>* getData() const;
    };
}

#endif //RAWDATA_H
