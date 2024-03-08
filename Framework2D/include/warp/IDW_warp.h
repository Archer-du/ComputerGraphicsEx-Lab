#pragma once

#include<warp/warp.h>

class WarperIDW : public ImageWarper
{
public:
    WarperIDW(
        int n,
        const std::vector<ImVec2>& starts,
        const std::vector<ImVec2>& ends,
        float mu)
        : ImageWarper(n, starts, ends),
          mu(mu) {}

    std::pair<int, int> warping(int x, int y) const override;

private:
    inline float sigma(int x, int y, float u, ImVec2 p_i) const
    {
        float norm =
            std::sqrt((p_i.x - x) * (p_i.x - x) + (p_i.y - y) * (p_i.y - y));
        float frac = std::pow(norm, u);
        return 1 / frac;
    }

private:
    const float mu;
};