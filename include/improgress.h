#ifndef IMPROGRESS_H
#define IMPROGRESS_H

#include "imgui.h"

namespace ImGui {
    // Helpers for displaying loading symbols
    bool BufferingBar(const char* label, float value, const ImVec2& size_arg, const ImU32& bg_col, const ImU32& fg_col);
    bool Spinner(const char* label, float radius, int thickness, const ImU32& color);
    bool CircleProgressBar(const char* label, float radius, float thickness, const ImU32& color, float progress);
} // namespace ImGui

#endif // IMPROGRESS_H
