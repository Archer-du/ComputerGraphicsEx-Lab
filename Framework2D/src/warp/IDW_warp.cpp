#include "warp/IDW_warp.h"
#include <cmath>

std::pair<int, int> WarperIDW::warping(int x, int y) const
{
    //TODO: config
    const float u = 1;

    std::pair<float, float> sum(0, 0);
    float deno = 0;
    for (int j = 0; j < n; j++)
    {
        deno += sigma(x, y, u, starts[j]);
    }
    for (int i = 0; i < n; i++)
    {
        float w = sigma(x, y, u, starts[i]) / deno;
        sum.first += w * (ends[i].x - starts[i].x);
        sum.second += w * (ends[i].y - starts[i].y);
    }
    return std::pair<int, int>(
        static_cast<int>(sum.first + x), static_cast<int>(sum.second + y));
}

float WarperIDW::sigma(int x, int y, float u, ImVec2 p_i) const
{
    float norm =
        std::sqrt((p_i.x - x) * (p_i.x - x) + (p_i.y - y) * (p_i.y - y));
    float frac = std::pow(norm, u);
    return 1 / frac;
}
