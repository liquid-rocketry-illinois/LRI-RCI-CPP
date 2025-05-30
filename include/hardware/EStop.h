#ifndef ESTOP_H
#define ESTOP_H

// Hardware class for ESTOP state
namespace LRI::RCI {
    class EStop {
        // Is the target in estop...
        bool isStopped;

        EStop() = default;
        ~EStop() = default;

    public:
        // Get singleton instance
        static EStop* getInstance();

        // Trigger an estop
        void ESTOP();

        // Get state of estop
        [[nodiscard]] bool isEstopped() const;

        // Receive confirmation of estop
        void receiveRCPUpdate(bool isStopped);
    };
} // namespace LRI::RCI

#endif // ESTOP_H
