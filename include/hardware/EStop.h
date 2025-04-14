#ifndef ESTOP_H
#define ESTOP_H

namespace LRI::RCI {
    class EStop {
        bool isStopped;

        EStop() = default;
        ~EStop() = default;

    public:
        static EStop* getInstance();

        void ESTOP();
        [[nodiscard]] bool isEstopped() const;
        void receiveRCPUpdate(bool isStopped);
    };
}

#endif //ESTOP_H
