#pragma once

#include <warp/warp.h>
#include <Eigen/Sparse>
#include <Eigen/SparseLU>

class WarperRBF : public ImageWarper
{
   public:
    WarperRBF(
        int n,
        const std::vector<ImVec2>& starts,
        const std::vector<ImVec2>& ends);

    std::pair<int, int> warping(int x, int y) const override;

   private:
    inline float euclidean_distance(const ImVec2& p1, const ImVec2& p2) const
    {
        return std::sqrt(
            (p1.x - p2.x) * (p1.x - p2.x) + (p1.y - p2.y) * (p1.y - p2.y));
    }

    inline float radial_basis_function(float d, float c) const
    {
        //return std::exp(-d * d / (2 * c * c));
        return std::sqrt(d * d + 256 * 256);
    }

   private:
    const float sigma = 1;
    Eigen::SparseMatrix<float> coef;
};