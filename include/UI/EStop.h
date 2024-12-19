#ifndef ESTOP_H
#define ESTOP_H

#include "BaseUI.h"

namespace LRI::RCI {
    class EStop : public BaseUI {
        static EStop* instance;
        EStop();

    public:
        void render() override;
        static EStop* getInstance();
    };
}

#endif //ESTOP_H
