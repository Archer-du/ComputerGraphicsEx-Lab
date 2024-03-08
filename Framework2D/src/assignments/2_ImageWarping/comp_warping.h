#pragma once

#include "view/comp_image.h"
#include <warp/warp.h>

namespace USTC_CG
{
// Image component for warping and other functions
class CompWarping : public ImageEditor
{
   public:
    enum class WarperType
    {
        IDW = 0,
        RBF = 1
    };

    explicit CompWarping(const std::string& label, const std::string& filename);
    virtual ~CompWarping() noexcept = default;

    void draw() override;

    // Simple edit functions
    void invert();
    void mirror(bool is_horizontal, bool is_vertical);
    void gray_scale();
    void warping(WarperType type);
    void restore();

    // Point selecting interaction
    void enable_selecting(bool flag);
    void select_points();
    void init_selections();

    void set_idw_mu(float mu);
    void set_rbf_sigma(float sigma);
    void set_ann_sample_num(int num);
    void set_ann_index_tree_num(int num);

   private:
    std::shared_ptr<ImageWarper> warper_;
    // Store the original image data
    std::shared_ptr<Image> back_up_;
    // The selected point couples for image warping
    std::vector<ImVec2> start_points_, end_points_;

    ImVec2 start_, end_;
    bool flag_enable_selecting_points_ = false;
    bool draw_status_ = false;

   private:
    float idw_mu = 1;
    float rbf_sigma = 10;
    float ann_sample_num = 4;
    float ann_index_tree_num = 2;
    // legacy: A simple "fish-eye" warping function
    std::pair<int, int> fisheye_warping(int x, int y, int width, int height);
};

}  // namespace USTC_CG