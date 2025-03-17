#ifndef ESTOP_H
#define ESTOP_H

#include "BaseUI.h"

namespace LRI::RCI {

    // A simple window which will send an E-STOP packet when pushed
    class EStopViewer : public BaseUI {
        // Singleton instance
        static EStopViewer* instance;

        EStopViewer();

    public:
        // Get singleton instance
        static EStopViewer* getInstance();

        // Overridden render
        void render() override;
    };
}

#endif //ESTOP_H
