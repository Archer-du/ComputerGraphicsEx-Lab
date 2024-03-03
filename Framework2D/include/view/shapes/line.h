#pragma once

#include "shape.h"

namespace USTC_CG
{
class Line : public Shape
{
   public:
    Line() = default;

    // Constructor to initialize a line with start and end coordinates
    Line(ImVec4 color, float thickness,
        float start_point_x,
        float start_point_y)
        : Shape(color, thickness),
          start_point_x_(start_point_x),
          start_point_y_(start_point_y)
    {
        end_point_x_ = start_point_x;
        end_point_y_ = start_point_y;
    }

    virtual ~Line() = default;

    // Overrides draw function to implement line-specific drawing logic
    void draw(float offset_x, float offset_y) const override;

    // Overrides Shape's update function to adjust the end point during
    // interaction
    void update(float x, float y) override;

   private:
    const float start_point_x_, start_point_y_;
    float end_point_x_, end_point_y_;
};
}  // namespace USTC_CG
