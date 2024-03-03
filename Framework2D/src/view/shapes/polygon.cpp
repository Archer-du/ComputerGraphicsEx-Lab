#include <imgui.h>

#include "view/shapes/polygon.h"

namespace USTC_CG
{
// Draw the line using ImGui
void Polygon::draw(float offset_x, float offset_y) const
{
    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    draw_list->AddPolyline(
        vertCoords.data(),
        vertCoords.size(),
        ImGui::ColorConvertFloat4ToU32(color),
        true,
        thickness);
}

void Polygon::update(float x, float y)
{
    vertCoords.back().x = x;
    vertCoords.back().y = y;
}

void Polygon::click_callback(float x, float y)
{
    vertCoords.back().x = x;
    vertCoords.back().y = y;
    if(vertCoords.size() < max_vert_num) 
        vertCoords.emplace_back(x, y);
}
}  // namespace USTC_CG