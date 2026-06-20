#include "UI/style.h"

namespace LRI::RCI {
    namespace {
        float scaleFactor;
    }

    void setImGuiStyles() {  }

    void setScaleFactor(float scale) {
        scaleFactor = scale;
    }

    float operator""_sc(unsigned long long value) {
        return static_cast<float>(value) * scaleFactor;
    }

    float operator""_sc(long double value) {
        return static_cast<float>(value) * scaleFactor;
    }
} // namespace LRI::RCI
