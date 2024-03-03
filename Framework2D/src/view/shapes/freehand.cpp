#include <imgui.h>

#include "view/shapes/freehand.h"

namespace USTC_CG
{
// Draw the line using ImGui
void FreeHand::draw(float offset_x, float offset_y) const
{
    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    draw_list->AddPolyline(
        vertCoords.data(),
        vertCoords.size(),
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