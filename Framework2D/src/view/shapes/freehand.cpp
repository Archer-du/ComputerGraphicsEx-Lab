#include <imgui.h>

#include "view/shapes/freehand.h"

namespace USTC_CG
{
// Draw the line using ImGui
void FreeHand::draw(const Config& config) const
{
    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    draw_list->AddPolyline(
        vertCoords.data(),
        vertCoords.size(),
        IM_COL32(
            config.line_color[0],
            config.line_color[1],
            config.line_color[2],
            config.line_color[3]),
        false,
        config.line_thickness);
}

void FreeHand::drag_callback(float x, float y)
{
    vertCoords.back().x = x;
    vertCoords.back().y = y;
    vertCoords.emplace_back(x, y);
}
}  // namespace USTC_CG