#include "seamless_clone.h"

void SeamlessCloner::set_decomposer(std::shared_ptr<Predecomposer> decomposer)
{
    decomposer_ = decomposer;
}

void SeamlessCloner::set_target_image(std::shared_ptr<USTC_CG::Image> dst_image)
{
    dst_image_ = dst_image;
}

void SeamlessCloner::set_source_image(std::shared_ptr<USTC_CG::Image> src_image)
{
    src_image_ = src_image;
}

void SeamlessCloner::set_offset(int offset_x, int offset_y)
{
    offset_x_ = offset_x;
    offset_y_ = offset_y;
}

void SeamlessCloner::solve()
{
    color_vec.clear();
    int channels = src_image_->channels();

    for (auto channel = 0; channel < channels; ++channel)
    {
        Eigen::VectorXd b = Eigen::VectorXd::Zero(decomposer_->A_.rows());

        for (const auto& pair : decomposer_->index_map)
        {
            int i = pair.first.first, j = pair.first.second;
            int idx = pair.second;
            if (check_valid_range(i + offset_x_, j + offset_y_)) continue;

            // calculate gradient mix color channel
            double total = 0;
            double d_pivot = dst_image_->get_pixel(i + offset_x_, j + offset_y_)[channel];
            double s_pivot = src_image_->get_pixel(i, j)[channel];
            for (auto &neighbor : decomposer_->neighbor_map[idx])
            {
                if (check_valid_range(neighbor.first + offset_x_, neighbor.second + offset_y_)) continue;

                double d_neighbor = dst_image_->get_pixel(
                    neighbor.first + offset_x_, neighbor.second + offset_y_)[channel];
                double s_neighbor = src_image_->get_pixel(neighbor.first, neighbor.second)[channel];
                total += get_gradient_mix(d_pivot, d_neighbor, s_pivot, s_neighbor);
            }

            b(idx) = total;
            // smooth border
            for (auto &border : decomposer_->border_map[idx])
            {
                if (check_valid_range(border.first + offset_x_, border.second + offset_y_)) continue;

                b(idx) += dst_image_->get_pixel(
                    border.first + offset_x_, border.second + offset_y_)[channel];
            }
        }

        auto x = decomposer_->solver.solve(b);
        color_vec.push_back(x);
    }
}

std::vector<unsigned char> SeamlessCloner::get_pixel(int i, int j, int channels)
{
    std::vector<unsigned char> pixel;
    int channel = 0;
    for (channel = 0; channel < channels; ++channel)
    {
        pixel.push_back(static_cast<unsigned char>(std::clamp<double>(
            color_vec[channel](
                decomposer_->index_map[std::pair<int, int>(i, j)]),
            0,
            255)));
    }
    return pixel;
}
