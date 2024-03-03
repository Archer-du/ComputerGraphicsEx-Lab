#include "view/comp_canvas.h"

#include <cmath>
#include <iostream>

#include "imgui.h"
#include "view/shapes/line.h"
#include "view/shapes/rect.h"
#include <view/shapes/ellipse.h>
#include <view/shapes/polygon.h>
#include <view/shapes/freehand.h>

namespace USTC_CG
{
void Canvas::draw()
{
    ImGuiIO& io = ImGui::GetIO();
    mousePos = io.MousePos;

    mouse_poll_event();

    draw_background();
    draw_shapes();
}

void Canvas::set_attributes(const ImVec2& min, const ImVec2& size)
{
    canvas_min_ = min;
    canvas_size_ = size;
    canvas_minimal_size_ = size;
    canvas_max_ =
        ImVec2(canvas_min_.x + canvas_size_.x, canvas_min_.y + canvas_size_.y);
}

void Canvas::set_color(ImVec4 color)
{
    draw_color = color;
}

void Canvas::set_thickness(float thickness)
{
    draw_thickness = thickness;
}

void Canvas::set_tools_pen()
{
}

void Canvas::set_tools_eraser()
{
}

void Canvas::set_tools_hand()
{
}

void Canvas::set_tools_paint()
{
}

void Canvas::undo()
{
    // stack empty -- disable
    // 获取栈顶的操作
    Operation op = undo_stack.top();
    // 弹出栈顶的操作
    undo_stack.pop();
    // 根据操作的类型执行相反的操作
    switch (op.type)
    {
        case Insert:  // 如果是插入，就删除
            op.shape->enable = false;
            break;
        default: break;
    }
    // 将操作压入redo栈
    redo_stack.push(op);
}

void Canvas::redo()
{
    // 获取栈顶的操作
    Operation op = redo_stack.top();
    // 弹出栈顶的操作
    redo_stack.pop();
    // 根据操作的类型重新执行操作
    switch (op.type)
    {
        case Insert:  // 如果是插入，就插入
            op.shape->enable = true;
            break;
    }
    // 将操作压入undo栈
    undo_stack.push(op);
}

void Canvas::show_background(bool flag)
{
    show_background_ = flag;
}

void Canvas::set_default()
{
    shape_type_ = kDefault;
}

void Canvas::set_line()
{
    shape_type_ = kLine;
}

void Canvas::set_rect()
{
    shape_type_ = kRect;
}

void Canvas::set_ellipse()
{
    shape_type_ = kEllipse;
}

void Canvas::set_polygon()
{
    shape_type_ = kPolygon;
}

void Canvas::set_freehand()
{
    shape_type_ = kFreehand;
}

void Canvas::clear_shape_list()
{
    shape_list_.clear();
}

void Canvas::draw_background()
{
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    if (show_background_)
    {
        // Draw background recrangle
        draw_list->AddRectFilled(canvas_min_, canvas_max_, background_color_);
        // Draw background border
        draw_list->AddRect(canvas_min_, canvas_max_, border_color_);
    }
    /// Invisible button over the canvas to capture mouse interactions.
    ImGui::SetCursorScreenPos(canvas_min_);
    ImGui::InvisibleButton(
        label_.c_str(),
        canvas_size_,
        ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight);
    // Record the current status of the invisible button
    is_hovered_ = ImGui::IsItemHovered();
    is_active_ = ImGui::IsItemActive();
}

void Canvas::draw_shapes()
{
    //Shape::Config s = { .bias = { canvas_min_.x, canvas_min_.y } };
    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    // ClipRect can hide the drawing content outside of the rectangular area
    draw_list->PushClipRect(canvas_min_, canvas_max_, true);
    const float GRID_STEP = 64.0f;
    for (float x = fmodf(scrolling.x, GRID_STEP); x < canvas_size_.x;
            x += GRID_STEP)
        draw_list->AddLine(
            ImVec2(canvas_min_.x + x, canvas_min_.y),
            ImVec2(canvas_min_.x + x, canvas_max_.y),
            IM_COL32(200, 200, 200, 40));
    for (float y = fmodf(scrolling.y, GRID_STEP); y < canvas_size_.y;
            y += GRID_STEP)
        draw_list->AddLine(
            ImVec2(canvas_min_.x, canvas_min_.y + y),
            ImVec2(canvas_max_.x, canvas_min_.y + y),
            IM_COL32(200, 200, 200, 40));

    for (const auto& shape : shape_list_)
    {
        if(shape->enable) shape->draw(scrolling.x, scrolling.y);
    }
    if (draw_status_ && current_shape_)
    {
        current_shape_->draw(scrolling.x, scrolling.y);
    }
    draw_list->PopClipRect();
}


void Canvas::mouse_poll_event()
{
    if (is_hovered_ && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
    {
        left_click_event();
    }
    if (is_hovered_ && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
    {
        right_click_event();
    }
    if (is_active_ && ImGui::IsMouseDragging(ImGuiMouseButton_Left))
    {
        left_drag_event();
    }
    if (is_active_ && ImGui::IsMouseDragging(ImGuiMouseButton_Right))
    {
        right_drag_event();
    }
    if (is_hovered_ && ImGui::IsMouseReleased(ImGuiMouseButton_Left))
    {
        left_release_event();
    }
}
void Canvas::left_click_event()
{
    if (!draw_status_ && shape_type_ != kDefault)
    {
        draw_status_ = true;
        on_draw_start();
    }
    else if (current_shape_)
    {
        current_shape_->click_callback(mousePos.x, mousePos.y);
    }
}
void Canvas::right_click_event()
{
}
void Canvas::left_drag_event()
{
    if (current_shape_)
    {
        current_shape_->update(mousePos.x, mousePos.y);
    }
}
void Canvas::right_drag_event()
{
    const float mouse_threshold_for_pan = -1;
    ImGuiIO& io = ImGui::GetIO();
    scrolling.x += io.MouseDelta.x;
    scrolling.y += io.MouseDelta.y;
}
void Canvas::left_release_event()
{
    if (draw_status_ && shape_type_ != kPolygon)
    {
        draw_status_ = false;
        on_draw_stop();
    }
}



void Canvas::on_draw_start()
{
    switch (shape_type_)
    {
        case USTC_CG::Canvas::kDefault:
        {
            break;
        }
        case USTC_CG::Canvas::kLine:
        {
            current_shape_ = std::make_shared<Line>(
                draw_color, draw_thickness, mousePos.x, mousePos.y);
            break;
        }
        case USTC_CG::Canvas::kRect:
        {
            current_shape_ = std::make_shared<Rect>(
                draw_color, draw_thickness, mousePos.x, mousePos.y);
            break;
        }
        case USTC_CG::Canvas::kEllipse:
        {
            current_shape_ = std::make_shared<Ellipse>(
                draw_color, draw_thickness, mousePos.x, mousePos.y);
            break;
        }
        case USTC_CG::Canvas::kPolygon:
        {
            current_shape_ = std::make_shared<Polygon>(
                draw_color, draw_thickness, mousePos.x, mousePos.y);
            break;
        }
        case USTC_CG::Canvas::kFreehand:
        {
            current_shape_ = std::make_shared<FreeHand>(
                draw_color, draw_thickness, mousePos.x, mousePos.y);
            break;
        }
        default: break;
    }
}

void Canvas::on_draw_stop()
{
    if (current_shape_)
    {
        shape_list_.push_back(current_shape_);
        undo_stack.push({ current_shape_, Insert });
        while (!redo_stack.empty())
        {
            redo_stack.pop();
        }
        current_shape_.reset();
    }
}



ImVec2 Canvas::mouse_pos_in_canvas() const
{
    ImGuiIO& io = ImGui::GetIO();
    const ImVec2 mouse_pos_in_canvas(
        io.MousePos.x - canvas_min_.x, io.MousePos.y - canvas_min_.y);
    return mouse_pos_in_canvas;
}
}  // namespace USTC_CG