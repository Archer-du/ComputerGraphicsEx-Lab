#pragma once

#include <utility>
#include <vector>
#include <imgui.h>

class ImageWarper {
public:
    explicit ImageWarper(int width, int height) : width(width), height(height){}
    virtual ~ImageWarper() = default;

    virtual std::pair<int, int> warping(
        int x,
        int y,
        int n,
        const std::vector<ImVec2>& starts,
        const std::vector<ImVec2>& ends) const = 0;

private:
    int width, height;
};