#ifndef ESTOP_H
#define ESTOP_H

#include "BaseUI.h"

namespace LRI::RCI {

    // A simple window which will send an E-STOP packet when pushed
    class EStop : public BaseUI {
        // Singleton instance
        static EStop* instance;

        EStop();

    public:
        // Get singleton instance
        static EStop* getInstance();

        // Overridden render
        void render() override;
    };
}

#endif //ESTOP_H
