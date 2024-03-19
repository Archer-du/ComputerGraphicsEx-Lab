#pragma once

#include "comp_source_image.h"
#include "predecomposer.h"

class SeamlessCloner
{
   public:
    SeamlessCloner() = default;
    ~SeamlessCloner() = default;

    void set_decomposer(std::shared_ptr<Predecomposer> decomposer);
    void set_target_image(std::shared_ptr<USTC_CG::Image> dst_image);
    void set_source_image(std::shared_ptr<USTC_CG::Image> src_image);
    void set_offset(int offset_x, int offset_y);

    void solve();

    std::vector<unsigned char> get_pixel(int i, int j, int channels);

   private:
    inline double get_gradient_mix(double f_p, double f_q, double g_p, double g_q)
    {
        double v_pq_f = f_p - f_q;
        double v_pq_g = g_p - g_q;
        return (v_pq_f * v_pq_f) > (v_pq_g * v_pq_g) ? v_pq_f : v_pq_g;
    }

   private:
    inline bool check_valid_range(int i, int j)
    {
        return i >= dst_image_->width() ||
            j >= dst_image_->height();
    }

   private:
    int offset_x_;
    int offset_y_;
    std::shared_ptr<USTC_CG::Image> src_image_;
    std::shared_ptr<USTC_CG::Image> dst_image_;
    std::shared_ptr<Predecomposer> decomposer_;
    std::vector<Eigen::VectorXd> color_vec;
};
