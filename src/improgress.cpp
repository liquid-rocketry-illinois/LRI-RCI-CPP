#include "improgress.h"

#include "imgui.h"
#include "imgui_internal.h"

namespace ImGui {

    // Progress indicators from here: https://github.com/ocornut/imgui/issues/1901
    bool BufferingBar(const char* label, float value, const ImVec2& size_arg, const ImU32& bg_col,
                      const ImU32& fg_col) {
        ImGuiWindow* window = GetCurrentWindow();
        if(window->SkipItems) return false;

        ImGuiContext& g = *GImGui;
        const ImGuiStyle& style = g.Style;
        const ImGuiID id = window->GetID(label);

        ImVec2 pos = window->DC.CursorPos;
        ImVec2 size = size_arg;
        size.x -= style.FramePadding.x * 2;

        const ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));
        ItemSize(bb, style.FramePadding.y);
        if(!ItemAdd(bb, id)) return false;

        // Render
        const float circleStart = size.x * 0.7f;
        const float circleEnd = size.x;
        const float circleWidth = circleEnd - circleStart;

        window->DrawList->AddRectFilled(bb.Min, ImVec2(pos.x + circleStart, bb.Max.y), bg_col);
        window->DrawList->AddRectFilled(bb.Min, ImVec2(pos.x + circleStart * value, bb.Max.y), fg_col);

        return true;
    }

    bool Spinner(const char* label, float radius, int thickness, const ImU32& color) {
        ImGuiWindow* window = GetCurrentWindow();
        if(window->SkipItems) return false;

        ImGuiContext& g = *GImGui;
        const ImGuiStyle& style = g.Style;
        const ImGuiID id = window->GetID(label);

        ImVec2 pos = window->DC.CursorPos;
        ImVec2 size((radius) * 2, (radius + style.FramePadding.y) * 2);

        const ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));
        ItemSize(bb, style.FramePadding.y);
        if(!ItemAdd(bb, id)) return false;

        // Render
        window->DrawList->PathClear();

        int num_segments = 30;
        float val = ImSin(g.Time * 1.8f) * (num_segments - 5);
        int start = (int) (val < 0 ? -val : val);

        const float a_min = IM_PI * 2.0f * ((float) start) / (float) num_segments;
        const float a_max = IM_PI * 2.0f * ((float) num_segments - 3) / (float) num_segments;

        const ImVec2 centre = ImVec2(pos.x + radius, pos.y + radius + style.FramePadding.y);

        for(int i = 0; i < num_segments; i++) {
            const float a = a_min + ((float) i / (float) num_segments) * (a_max - a_min);
            window->DrawList->PathLineTo(ImVec2(centre.x + ImCos(a + g.Time * 8) * radius,
                                                centre.y + ImSin(a + g.Time * 8) * radius));
        }

        window->DrawList->PathStroke(color, false, thickness);

        return true;
    }

}
