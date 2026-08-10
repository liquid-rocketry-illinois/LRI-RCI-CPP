#include "util/FrameDeferred.h"

namespace LRI::RCI {
    std::map<void*, FrameDeferred::DeferInfo> FrameDeferred::deferredValues;

    void FrameDeferred::updateDeferredValues() {
        for(const auto& [location, info] : deferredValues) {
            info.mover(location, info.value);
            info.deleter(info.value);
        }

        deferredValues.clear();

        for(const auto& func : deferredFuncs) func();
        deferredFuncs.clear();
    }
} // namespace LRI::RCI
