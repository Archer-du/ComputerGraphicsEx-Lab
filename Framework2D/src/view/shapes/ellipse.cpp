#include <imgui.h>

#include "view/shapes/ellipse.h"
#include <cmath>

namespace USTC_CG
{
// Draw the line using ImGui
void Ellipse::draw() const
{
    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    draw_list->AddEllipse(
        ImVec2(
            (start_point_x_ + end_point_x_) / 2 + offset_x_,
            (start_point_y_ + end_point_y_) / 2 + offset_y_),
        fabsf((end_point_x_ - start_point_x_)) / 2,
        fabsf((end_point_y_ - start_point_y_)) / 2,
        ImGui::ColorConvertFloat4ToU32(color),
        0,
        0,
        thickness);
}

void Ellipse::update(float x, float y)
{
    end_point_x_ = x;
    end_point_y_ = y;
}
}  // namespace USTC_CG