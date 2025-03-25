#ifndef RAWVIEWER_H
#define RAWVIEWER_H

#include <sstream>

#include "BaseUI.h"
#include "hardware/RawData.h"

namespace LRI::RCI {
    // Window which shows the output to the custom device class from RCP
    class RawViewer : public BaseUI {
        const ImVec2 size;

    public:
        explicit RawViewer(const ImVec2&& size);
        ~RawViewer() override = default;

        // Overridden render function
        void render() override;
    };
}

#endif //RAWVIEWER_H
