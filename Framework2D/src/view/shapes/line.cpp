#include "view/shapes/line.h"

#include <imgui.h>

namespace USTC_CG
{
// Draw the line using ImGui
void Line::draw(float offset_x, float offset_y) const
{
    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    draw_list->AddLine(
        ImVec2(start_point_x_ + offset_x, start_point_y_ + offset_y),
        ImVec2(end_point_x_ + offset_x, end_point_y_ + offset_y),
        ImGui::ColorConvertFloat4ToU32(color),
        thickness);
}

void Line::update(float x, float y)
{
    end_point_x_ = x;
    end_point_y_ = y;
}
}  // namespace USTC_CG