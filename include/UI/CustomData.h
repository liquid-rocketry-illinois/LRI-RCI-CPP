#ifndef CUSTOMDATA_H
#define CUSTOMDATA_H

#include <vector>
#include <sstream>

#include "BaseUI.h"
#include "RCP_Host/RCP_Host.h"

namespace LRI::RCI {

    // Window which shows the output to the custom device class from RCP
    class CustomData : public BaseUI {
        // Singleton instance
        static CustomData* instance;

        // Size of buffer the user can type in to send data back
        static constexpr size_t OUT_SIZE = 64;

        // How incomming data should be interpreted: as hex data, decimal data, or as a string
        enum class InterpretMode {
            HEX,
            DEC,
            STR
        };

        // Helper to convert InterpretMode enum to string
        static std::string interpretModeToString(InterpretMode mode);

        // Helper to format all data in the raw buffer and write it to an ostream, considering the current interpret mode
        static void formatRaw(std::basic_ostream<char>& out, std::vector<uint8_t>& raw, const InterpretMode& mode,
                              int* elems = nullptr);

        // Vector to store raw bytes
        std::vector<uint8_t> raw;

        // Stringstream that gets appended to and holds the correctly formatted data
        std::stringstream display;

        // Current interpretation mode
        InterpretMode mode;

        // Buffer the user can write data into and send back to the target
        char out[OUT_SIZE];

        // Used for formatting
        int numElems;

        CustomData();

    public:
        // Get singleton instance
        static CustomData* getInstance();

        // Overridden render function
        void render() override;

        // Used as callback for RCP
        void recevieRCPUpdate(const RCP_CustomData& data);

        // Needs a custom reset
        void reset() override;

        ~CustomData() override = default;
    };
}

#endif //CUSTOMDATA_H
