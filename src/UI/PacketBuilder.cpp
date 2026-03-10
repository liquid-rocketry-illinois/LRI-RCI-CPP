#include "UI/PacketBuilder.h"

#include "imgui.h"

namespace LRI::RCI::PKTB {
    namespace {
        constexpr ImGuiWindowFlags PKTB_WFLAGS = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoDocking;
    }

    void render(const Box& windowRegion) {
        ImGui::SetNextWindowPos(windowRegion.tl());
        ImGui::SetNextWindowSize(windowRegion.size());
        ImGui::Begin("Packet Builder", nullptr, PKTB_WFLAGS);
        ImGui::Text("eee");
        ImGui::End();
    }
} // namespace LRI::RCI::PKTB
