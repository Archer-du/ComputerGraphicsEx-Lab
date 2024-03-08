#pragma once

#include <utility>
#include <vector>
#include <imgui.h>
#include <iostream>

class ImageWarper {
public:
    explicit ImageWarper(
        int n,
        const std::vector<ImVec2>& starts,
        const std::vector<ImVec2>& ends)
        : starts(starts),
        ends(ends),
        n(n) {
    }

    virtual ~ImageWarper() = default;

    virtual std::pair<int, int> warping(int x, int y) const = 0;

protected:
    inline float euclidean_distance(const ImVec2& p1, const ImVec2& p2) const
    {
        return std::sqrt(
            (p1.x - p2.x) * (p1.x - p2.x) + (p1.y - p2.y) * (p1.y - p2.y));
    }

   protected:
    const int n;
    const std::vector<ImVec2> starts;
    const std::vector<ImVec2> ends;
};