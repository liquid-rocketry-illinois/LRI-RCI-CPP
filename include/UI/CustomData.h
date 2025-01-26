#ifndef CUSTOMDATA_H
#define CUSTOMDATA_H

#include <vector>
#include <sstream>

#include "BaseUI.h"
#include "RCP_Host/RCP_Host.h"

namespace LRI::RCI {
    class CustomData : public BaseUI {
        static CustomData* instance;
        static constexpr size_t OUT_SIZE = 64;

        enum class InterpretMode {
            HEX,
            DEC,
            STR
        };

        static std::string interpretModeToString(InterpretMode mode);
        static void formatRaw(std::basic_ostream<char>& out, std::vector<uint8_t>& raw, const InterpretMode& mode,
                                    int* elems = nullptr);

        std::vector<uint8_t> raw;
        std::stringstream display;
        InterpretMode mode;
        char out[OUT_SIZE];
        int numElems;


        CustomData();

    public:
        static CustomData* getInstance();

        void render() override;
        void recevieRCPUpdate(const RCP_CustomData& data);

        ~CustomData() override = default;
    };
}

#endif //CUSTOMDATA_H
