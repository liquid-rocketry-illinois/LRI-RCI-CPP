#ifndef RAWDATA_H
#define RAWDATA_H

#include <sstream>

#include "RCP_Host/RCP_Host.h"

namespace LRI::RCI {
    class RawData {
        RawData() = default;
        ~RawData() = default;

        std::stringstream chars;

    public:
        static RawData* getInstance();

        void receiveRCPUpdate(const RCP_CustomData& data);
        void reset();

        [[nodiscard]] const std::stringstream& getData() const;
    };
}

#endif //RAWDATA_H
