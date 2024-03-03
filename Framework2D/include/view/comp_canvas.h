#pragma once

#include <imgui.h>

#include <memory>
#include <vector>

#include "shapes/shape.h"
#include "view/component.h"
#include <stack>

namespace USTC_CG
{

// Canvas class for drawing shapes.
class Canvas : public Component
{
   public:
    // Inherits constructor from Component.
    using Component::Component;

    // Override the draw method from the parent Component class.
    void draw() override;

    // Enumeration for supported shape types.
    enum ShapeType
    {
        kDefault = 0,
        kLine = 1,
        kRect = 2,
        kEllipse = 3,
        kPolygon = 4,
        kFreehand = 5,
    };
    enum OperateType
    {
        Insert
    };

    struct Operation
    {
        std::shared_ptr<Shape> shape;
        OperateType type;
    };

    // Shape type setters.
    void set_default();
    void set_line();
    void set_rect();
    void set_ellipse();
    void set_polygon();
    void set_freehand();

    // Clears all shapes from the canvas.
    void clear_shape_list();

    // Set canvas attributes (position and size).
    void set_attributes(const ImVec2& min, const ImVec2& size);

    void set_color(ImVec4 color);
    void set_thickness(float thickness);

    void set_tools_pen();
    void set_tools_eraser();
    void set_tools_hand();
    void set_tools_paint();

    void undo();
    void redo();

    // Controls the visibility of the canvas background.
    void show_background(bool flag);

   private:
    // Drawing functions.
    void draw_background();
    void draw_shapes();

    // Event handlers for mouse interactions.
    void mouse_poll_event();
    void left_click_event();
    void right_click_event();
    void left_drag_event();
    void right_drag_event();
    void left_release_event();

    void on_draw_start();
    void on_draw_stop();

    // Calculates mouse's relative position in the canvas.
    ImVec2 mouse_pos_in_canvas() const;

    // Canvas attributes.
    ImVec2 canvas_min_;         // Top-left corner of the canvas.
    ImVec2 canvas_max_;         // Bottom-right corner of the canvas.
    ImVec2 canvas_size_;        // Size of the canvas.
    ImVec2 scrolling;
    bool draw_status_ = false;  // Is the canvas currently being drawn on.

    ImVec4 draw_color;
    float draw_thickness;

    ImVec2 canvas_minimal_size_ = ImVec2(50.f, 50.f);
    ImU32 background_color_ = IM_COL32(50, 50, 50, 255);
    ImU32 border_color_ = IM_COL32(255, 255, 255, 255);
    bool show_background_ = true;  // Controls background visibility.

    // Mouse interaction status.
    bool is_hovered_, is_active_;

    // Current shape being drawn.
    ShapeType shape_type_ = kDefault;
    ImVec2 mousePos;
    std::shared_ptr<Shape> current_shape_;

    // List of shapes drawn on the canvas.
    std::vector<std::shared_ptr<Shape>> shape_list_;

    std::stack<Operation> undo_stack;
    std::stack<Operation> redo_stack;
};

}  // namespace USTC_CG
