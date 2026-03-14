#include "UI/PacketBuilder.h"

#include "imgui.h"

namespace LRI::RCI::PKTB {

    void render(const Box& windowRegion) {
        ImGui::SetNextWindowPos(windowRegion.tl());
        ImGui::SetNextWindowSize(windowRegion.size());
        ImGui::Begin("##packetbuilder", nullptr, WFLAGS);
        ImGui::Text("eee");
        ImGui::End();
    }
} // namespace LRI::RCI::PKTB
