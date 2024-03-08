#include "annoylib.h"
#include "kissrandom.h"

#include "comp_warping.h"

#include <cmath>
#include <warp/IDW_warp.h>
#include <warp/RBF_warp.h>

namespace USTC_CG
{
using uchar = unsigned char;

CompWarping::CompWarping(const std::string& label, const std::string& filename)
    : ImageEditor(label, filename)
{
    if (data_)
        back_up_ = std::make_shared<Image>(*data_);
}

void CompWarping::draw()
{
    // Draw the image
    ImageEditor::draw();
    // Draw the canvas
    if (flag_enable_selecting_points_)
        select_points();
}

void CompWarping::invert()
{
    for (int i = 0; i < data_->width(); ++i)
    {
        for (int j = 0; j < data_->height(); ++j)
        {
            const auto color = data_->get_pixel(i, j);
            data_->set_pixel(
                i,
                j,
                { static_cast<uchar>(255 - color[0]),
                  static_cast<uchar>(255 - color[1]),
                  static_cast<uchar>(255 - color[2]) });
        }
    }
    // After change the image, we should reload the image data to the renderer
    update();
}
void CompWarping::mirror(bool is_horizontal, bool is_vertical)
{
    Image image_tmp(*data_);
    int width = data_->width();
    int height = data_->height();

    if (is_horizontal)
    {
        if (is_vertical)
        {
            for (int i = 0; i < width; ++i)
            {
                for (int j = 0; j < height; ++j)
                {
                    data_->set_pixel(
                        i,
                        j,
                        image_tmp.get_pixel(width - 1 - i, height - 1 - j));
                }
            }
        }
        else
        {
            for (int i = 0; i < width; ++i)
            {
                for (int j = 0; j < height; ++j)
                {
                    data_->set_pixel(
                        i, j, image_tmp.get_pixel(width - 1 - i, j));
                }
            }
        }
    }
    else
    {
        if (is_vertical)
        {
            for (int i = 0; i < width; ++i)
            {
                for (int j = 0; j < height; ++j)
                {
                    data_->set_pixel(
                        i, j, image_tmp.get_pixel(i, height - 1 - j));
                }
            }
        }
    }

    // After change the image, we should reload the image data to the renderer
    update();
}
void CompWarping::gray_scale()
{
    for (int i = 0; i < data_->width(); ++i)
    {
        for (int j = 0; j < data_->height(); ++j)
        {
            const auto color = data_->get_pixel(i, j);
            uchar gray_value = (color[0] + color[1] + color[2]) / 3;
            data_->set_pixel(i, j, { gray_value, gray_value, gray_value });
        }
    }
    // After change the image, we should reload the image data to the renderer
    update();
}

void CompWarping::warping(WarperType type)
{
    // HW2_TODO: You should implement your own warping function that interpolate
    // the selected points.
    // You can design a class for such warping operations, utilizing the
    // encapsulation, inheritance, and polymorphism features of C++. More files
    // like "*.h", "*.cpp" can be added to this directory or anywhere you like.
    int n = start_points_.size();
    if (n <= 0) return;

    switch (type)
    {
        case WarperType::IDW:
        {
            warper_ = std::make_shared<WarperIDW>
                (n, start_points_, end_points_, idw_mu);
            break;
        }
        case WarperType::RBF:
        {
            warper_ = std::make_shared<WarperRBF>
                (n, start_points_, end_points_, rbf_sigma);
            break;
        }
        default: break;
    }

    // Create a new image to store the result
    Image warped_image(*data_);
    // Initialize the color of result image
    for (int y = 0; y < data_->height(); ++y)
    {
        for (int x = 0; x < data_->width(); ++x)
        {
            warped_image.set_pixel(x, y, { 0, 0, 0 });
        }
    }
    
    Annoy::AnnoyIndex<
        int,
        float,
        Annoy::Euclidean,
        Annoy::Kiss32Random,
        Annoy::AnnoyIndexSingleThreadedBuildPolicy>
        index(2);
    float i = 0;

    // Example: (simplified) "fish-eye" warping
    // For each (x, y) from the input image, the "fish-eye" warping transfer it
    // to (x', y') in the new image:
    // Note: For this transformation ("fish-eye" warping), one can also
    // calculate the inverse (x', y') -> (x, y) to fill in the "gaps".
    float height = data_->height();
    float width = data_->width();
    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            // Apply warping function to (x, y), and we can get (x', y')
            auto [new_x, new_y] = warper_->warping(x, y);
            // Copy the color from the original image to the result image
            if (new_x >= 0 && new_x < width && new_y >= 0 && new_y < height)
            {
                std::vector<unsigned char> pixel = data_->get_pixel(x, y);
                warped_image.set_pixel(new_x, new_y, pixel);

                const float vec[2] = { (2 * new_x - width) / width,
                                       (2 * new_y - height) / height };
                index.add_item(i, vec);
                i++;
            }
        }
    }

    //ANN 补洞
    index.build(ann_index_tree_num);

    for (int y = 0; y < data_->height(); ++y)
    {
        for (int x = 0; x < data_->width(); ++x)
        {
            std::vector<unsigned char> pixel = warped_image.get_pixel(x, y);
            if (pixel[0] == 0 and pixel[1] == 0 and pixel[2] == 0)
            {
                float vec[2] = { (2 * x - width) / width,
                                 (2 * y - height) / height };
                std::vector<int> closest_items;
                std::vector<float> distances;
                index.get_nns_by_vector(vec, ann_sample_num, -1, &closest_items, &distances);
                //get average
                std::vector<unsigned char> channels(3, 0);
                for (int j = 0; j < ann_sample_num; j++)
                {
                    float result[2];
                    index.get_item(closest_items[j], result);
                    std::vector<unsigned char> sample = warped_image.get_pixel(
                        (result[0] * width + width) / 2,
                        (result[1] * height + height) / 2);
                    for (int i = 0; i < 3; i++)
                    {
                        channels[i] += sample[i] / ann_sample_num;
                    }
                }
                warped_image.set_pixel(x, y, channels);
            }
        }
    }

    *data_ = std::move(warped_image);
    update();
}
void CompWarping::restore()
{
    *data_ = *back_up_;
    update();
}
void CompWarping::enable_selecting(bool flag)
{
    flag_enable_selecting_points_ = flag;
}
void CompWarping::select_points()
{
    /// Invisible button over the canvas to capture mouse interactions.
    ImGui::SetCursorScreenPos(position_);
    ImGui::InvisibleButton(
        label_.c_str(),
        ImVec2(
            static_cast<float>(image_width_),
            static_cast<float>(image_height_)),
        ImGuiButtonFlags_MouseButtonLeft);
    // Record the current status of the invisible button
    bool is_hovered_ = ImGui::IsItemHovered();
    // Selections
    ImGuiIO& io = ImGui::GetIO();
    if (is_hovered_ && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
    {
        draw_status_ = true;
        start_ = end_ =
            ImVec2(io.MousePos.x - position_.x, io.MousePos.y - position_.y);
    }
    if (draw_status_)
    {
        end_ = ImVec2(io.MousePos.x - position_.x, io.MousePos.y - position_.y);
        if (!ImGui::IsMouseDown(ImGuiMouseButton_Left))
        {
            start_points_.push_back(start_);
            end_points_.push_back(end_);
            draw_status_ = false;
        }
    }
    // Visualization
    auto draw_list = ImGui::GetWindowDrawList();
    for (size_t i = 0; i < start_points_.size(); ++i)
    {
        ImVec2 s(
            start_points_[i].x + position_.x, start_points_[i].y + position_.y);
        ImVec2 e(
            end_points_[i].x + position_.x, end_points_[i].y + position_.y);
        draw_list->AddLine(s, e, IM_COL32(255, 0, 0, 255), 2.0f);
        draw_list->AddCircleFilled(s, 4.0f, IM_COL32(0, 0, 255, 255));
        draw_list->AddCircleFilled(e, 4.0f, IM_COL32(0, 255, 0, 255));
    }
    if (draw_status_)
    {
        ImVec2 s(start_.x + position_.x, start_.y + position_.y);
        ImVec2 e(end_.x + position_.x, end_.y + position_.y);
        draw_list->AddLine(s, e, IM_COL32(255, 0, 0, 255), 2.0f);
        draw_list->AddCircleFilled(s, 4.0f, IM_COL32(0, 0, 255, 255));
    }
}
void CompWarping::init_selections()
{
    start_points_.clear();
    end_points_.clear();
}

void CompWarping::set_idw_mu(float mu)
{
    idw_mu = mu;
}

void CompWarping::set_rbf_sigma(float sigma)
{
    rbf_sigma = sigma;
}

void CompWarping::set_ann_sample_num(int num)
{
    ann_sample_num = num;
}
void CompWarping::set_ann_index_tree_num(int num)
{
    ann_index_tree_num = num;
}




//legacy
std::pair<int, int>
CompWarping::fisheye_warping(int x, int y, int width, int height)
{
    float center_x = width / 2.0f;
    float center_y = height / 2.0f;
    float dx = x - center_x;
    float dy = y - center_y;
    float distance = std::sqrt(dx * dx + dy * dy);

    // Simple non-linear transformation r -> r' = f(r)
    float new_distance = std::sqrt(distance) * 10;

    if (distance == 0)
    {
        return { static_cast<int>(center_x), static_cast<int>(center_y) };
    }
    // (x', y')
    float ratio = new_distance / distance;
    int new_x = static_cast<int>(center_x + dx * ratio);
    int new_y = static_cast<int>(center_y + dy * ratio);

    return { new_x, new_y };
}
}  // namespace USTC_CG