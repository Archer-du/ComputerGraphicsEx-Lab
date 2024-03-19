#include <imgui.h>

#include "view/shapes/polygon.h"

namespace USTC_CG
{
// Draw the line using ImGui
void Polygon::draw() const
{
    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    std::vector<ImVec2> temp(vertCoords);
    for (auto& vec : temp)
    {
        vec.x += offset_x_;
        vec.y += offset_y_;
    }

    draw_list->AddPolyline(
        temp.data(),
        temp.size(),
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