#include <imgui.h>

#include "view/shapes/freehand.h"

namespace USTC_CG
{
// Draw the line using ImGui
void FreeHand::draw() const
{
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    std::vector<ImVec2> temp(vertCoords);
    for (auto& vec : temp)
    {
        vec.x += offset_x;
        vec.y += offset_y;
    }

    draw_list->AddPolyline(
        temp.data(),
        temp.size(),
        ImGui::ColorConvertFloat4ToU32(color),
        false,
        thickness);
}

void FreeHand::update(float x, float y)
{
    vertCoords.back().x = x;
    vertCoords.back().y = y;
    vertCoords.emplace_back(x, y);
}
}  // namespace USTC_CG