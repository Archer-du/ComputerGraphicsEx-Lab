#pragma once

#include "shape.h"
#include <imgui.h>
#include <vector>

namespace USTC_CG
{
class Polygon : public Shape
{
   public:
    Polygon() = default;

    // Constructor to initialize a line with start and end coordinates
    Polygon(
        float start_point_x,
        float start_point_y)
    {
        vertCoords.reserve(max_vert_num);
        vertCoords.emplace_back(start_point_x, start_point_y);
        vertCoords.emplace_back(start_point_x, start_point_y);
    }

    virtual ~Polygon() = default;

    // Overrides draw function to implement line-specific drawing logic
    void draw(const Config& config) const override;

    // Overrides Shape's update function to adjust the end point during
    // interaction
    void update(float x, float y) override;

    void click_callback(float x, float y) override;

   private:
    static const int max_vert_num = 20;
    //ImVec2 vertCoords[max_vert_num];
    std::vector<ImVec2> vertCoords;
};
}  // namespace USTC_CG
