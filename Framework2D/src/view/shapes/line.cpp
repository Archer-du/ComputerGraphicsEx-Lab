#include "view/shapes/line.h"

#include <imgui.h>

namespace USTC_CG
{
// Draw the line using ImGui
void Line::draw() const
{
    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    draw_list->AddLine(
        ImVec2(start_point_x_ + offset_x_, start_point_y_ + offset_y_),
        ImVec2(end_point_x_ + offset_x_, end_point_y_ + offset_y_),
        ImGui::ColorConvertFloat4ToU32(color),
        thickness);
}

void Line::update(float x, float y)
{
    end_point_x_ = x;
    end_point_y_ = y;
}
}  // namespace USTC_CG