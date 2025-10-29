#ifndef ESTOP_H
#define ESTOP_H

// Hardware class for ESTOP state
namespace LRI::RCI::EStop {
    // Trigger an estop
    void ESTOP();

    // Get state of estop
    [[nodiscard]] bool isEstopped();

    // Receive confirmation of estop
    void receiveRCPUpdate(bool isStopped);
} // namespace LRI::RCI::EStop

#endif // ESTOP_H
