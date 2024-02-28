#include <imgui.h>

#include "view/shapes/Polygon.h"

namespace USTC_CG
{
// Draw the line using ImGui
void Polygon::draw(const Config& config) const
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
        true,
        config.line_thickness);
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