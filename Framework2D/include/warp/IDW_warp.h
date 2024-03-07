#pragma once

#include<warp/warp.h>

class WarperIDW : public ImageWarper
{
public:
    using ImageWarper::ImageWarper;
    std::pair<int, int> warping(
        int x, int y,
        int n, const std::vector<ImVec2>& starts, const std::vector<ImVec2>& ends) const override;

private:
    float weight(int x, int y, int i, int n, float u, const std::vector<ImVec2>& starts) const;
    float sigma(int x, int y, float u, ImVec2 p_i) const;
};