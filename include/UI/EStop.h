#ifndef ESTOP_H
#define ESTOP_H

#include "BaseUI.h"

namespace LRI::RCI {
    class EStop : public BaseUI {
        static EStop* instance;
        EStop();

    public:
        void render() override;
        static const EStop* getInstance();
        void destroy() override;
    };
}

#endif //ESTOP_H
