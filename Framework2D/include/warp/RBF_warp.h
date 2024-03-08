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
        const std::vector<ImVec2>& ends, 
        float sigma);

    std::pair<int, int> warping(int x, int y) const override;

   private:
    inline float radial_basis_function(float d, float c) const
    {
        return std::sqrt(d * d + c * c);
    }

   private:
    const float sigma;
    Eigen::SparseMatrix<float> coef;
};