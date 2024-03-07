#pragma once

#include<warp/warp.h>

class WarperIDW : public ImageWarper
{
public:
    using ImageWarper::ImageWarper;

    std::pair<int, int> warping(int x, int y) const override;

private:
    float sigma(int x, int y, float u, ImVec2 p_i) const;
};