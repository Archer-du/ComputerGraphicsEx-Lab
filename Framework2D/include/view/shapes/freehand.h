#pragma once

#include <imgui.h>

#include <vector>

#include "shape.h"

namespace USTC_CG
{
class FreeHand : public Shape
{
   public:
    FreeHand() = default;

    // Constructor to initialize a line with start and end coordinates
    FreeHand(float start_point_x, float start_point_y)
    {
        vertCoords.emplace_back(start_point_x, start_point_y);
        vertCoords.emplace_back(start_point_x, start_point_y);
    }

    virtual ~FreeHand() = default;

    // Overrides draw function to implement line-specific drawing logic
    void draw(const Config& config) const override;
    void update(float x, float y) override {};

    void drag_callback(float x, float y) override;
   private:
    std::vector<ImVec2> vertCoords;
};
}  // namespace USTC_CG
