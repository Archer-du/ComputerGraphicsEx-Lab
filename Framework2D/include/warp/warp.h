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
    const int n;
    const std::vector<ImVec2> starts;
    const std::vector<ImVec2> ends;
};