#pragma once

#include <Eigen/Sparse>
#include <Eigen/SparseCholesky>
#include <iostream>
#include <map>
#include <vector>

#include "view/image.h"
#include <functional>

// 定义自定义哈希函数
struct pair_hash
{
    size_t operator()(const std::pair<int, int> &pair) const
    {
        return std::hash<int>()(pair.first) ^ std::hash<int>()(pair.second);
    }
};

class Predecomposer
{
   public:
    Predecomposer(std::shared_ptr<USTC_CG::Image> mask) : mask_(mask){}
    ~Predecomposer() = default;

    void solve();

   public:
    std::shared_ptr<USTC_CG::Image> mask_;

    Eigen::SparseMatrix<double> A_;

    // pos to index map
    std::unordered_map<std::pair<int, int>, int, pair_hash> index_map;
    // index to neighbors pos map
    std::unordered_map<int, std::vector<std::pair<int, int>>> neighbor_map;
    // index to borders pos (exclude, if exists) map
    std::unordered_map<int, std::vector<std::pair<int, int>>> border_map;

    Eigen::SimplicialLLT<Eigen::SparseMatrix<double>> solver;
};
