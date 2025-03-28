#ifndef SOLENOIDSVIEWER_H
#define SOLENOIDSVIEWER_H

#include <map>
#include <set>
#include "WModule.h"
#include "hardware/HardwareQualifier.h"
#include "hardware/Solenoids.h"

namespace LRI::RCI {
    // A window for showing and controlling solenoid status
    class SolenoidViewer : public WModule {
        static int CLASSID;

        const int classid;
        const bool refreshButton;
        // Maps human identifiable names to solenoid ID
        std::map<HardwareQualifier, const Solenoids::SolenoidState*> sols;

    public:
        explicit SolenoidViewer(const std::set<HardwareQualifier>&& quals, bool refreshButton = true);

        // Overridden render function
        void render() override;

        ~SolenoidViewer() override = default;
    };
}

#endif //SOLENOIDSVIEWER_H
