#include "view/shapes/rect.h"

#include <imgui.h>

namespace USTC_CG
{
// Draw the rectangle using ImGui
void Rect::draw(float offset_x, float offset_y) const
{
    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    draw_list->AddRect(
        ImVec2(start_point_x_ + offset_x, start_point_y_ + offset_y),
        ImVec2(end_point_x_ + offset_x, end_point_y_ + offset_y),
        ImGui::ColorConvertFloat4ToU32(color),
        0.f,  // No rounding of corners
        ImDrawFlags_None,
        thickness);
}

void Rect::update(float x, float y)
{
    end_point_x_ = x;
    end_point_y_ = y;
}
}  // namespace USTC_CG
