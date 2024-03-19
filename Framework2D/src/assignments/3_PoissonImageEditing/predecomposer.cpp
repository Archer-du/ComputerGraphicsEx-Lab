#include "predecomposer.h"

void Predecomposer::solve()
{
    index_map.clear();
    neighbor_map.clear();
    border_map.clear();

    int counter = 0;
    for (int i = 0; i < mask_->width(); ++i)
    {
        for (int j = 0; j < mask_->height(); ++j)
        {
            if (mask_->get_pixel(i, j)[0] > 0)
            {
                index_map.emplace(std::pair<int, int>(i, j), counter);
                border_map.emplace(
                    counter, std::vector<std::pair<int, int>>(0));
                counter++;
            }
        }
    }

    A_ = Eigen::SparseMatrix<double>(counter, counter);

    for (int i = 0; i < mask_->width(); ++i)
    {
        for (int j = 0; j < mask_->height(); ++j)
        {
            if (mask_->get_pixel(i, j)[0] > 0)
            {
                int idx = index_map[std::pair<int, int>(i, j)];
                int neighbor_count = 0;
                std::vector<std::pair<int, int>> near = {
                    { i - 1, j }, { i + 1, j }, { i, j - 1 }, { i, j + 1 }
                };

                for (const auto& n : near)
                {
                    int near_x = n.first;
                    int near_y = n.second;
                    if (0 <= near_x && near_x < mask_->width() && 0 <= near_y &&
                        near_y < mask_->height())
                    {
                        neighbor_map[idx].push_back(
                            std::pair<int, int>(near_x, near_y));
                        if (mask_->get_pixel(near_x, near_y)[0] > 0)
                            A_.coeffRef(idx, index_map[{ near_x, near_y }]) =
                                -1;
                        else
                            border_map[idx].push_back(
                                std::pair<int, int>(near_x, near_y));
                        ++neighbor_count;
                    }
                }
                A_.coeffRef(idx, idx) = neighbor_count;
            }
        }
    }
    A_.makeCompressed();

    std::cout << "decomposing" << std::endl;
    solver.compute(A_);

    if (solver.info() != Eigen::Success)
    {
        std::cout << "err" << std::endl;
    }
    else
    {
        std::cout << "ok" << std::endl;
    }
}
