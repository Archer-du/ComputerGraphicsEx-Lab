#include "window_minidraw.h"
#include <ImGuiFileDialog.h>

#include <iostream>

namespace USTC_CG
{
MiniDraw::MiniDraw(const std::string& window_name) : Window(window_name)
{
    p_canvas_ = std::make_shared<Canvas>("Cmpt.Canvas");
    p_canvas_->set_color(ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
    p_canvas_->set_thickness(2);
}

MiniDraw::~MiniDraw()
{
    p_canvas_.reset();
}

void MiniDraw::draw()
{
    draw_canvas();
    if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Z)) &&
            ImGui::GetIO().KeyCtrl)
    {
        p_canvas_->undo();
    }
    if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Y)) &&
            ImGui::GetIO().KeyCtrl)
    {
        p_canvas_->redo();
    }
}

void MiniDraw::draw_canvas()
{
    // Set a full screen canvas view
    const ImGuiViewport* viewport = ImGui::GetMainViewport();

    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);

    static ToolType show_config_window = kNone;

    static bool flag_open_file_dialog_ = false;
    if (flag_open_file_dialog_)
    {
        IGFD::FileDialogConfig config;
        config.path = ".";
        ImGuiFileDialog::Instance()->OpenDialog(
            "ChooseImageOpenFileDlg", "Choose Image File", ".png,.jpg", config);
        if (ImGuiFileDialog::Instance()->Display("ChooseImageOpenFileDlg"))
        {
            if (ImGuiFileDialog::Instance()->IsOk())
            {
                std::string filePathName =
                    ImGuiFileDialog::Instance()->GetFilePathName();
                std::string label = filePathName;
                p_image_ = std::make_shared<Image>(label, filePathName);
                p_canvas_->clear_shape_list();
            }
            ImGuiFileDialog::Instance()->Close();
            flag_open_file_dialog_ = false;
        }
    }
    if (ImGui::Begin(
            "Canvas",
            &flag_show_canvas_view_,
            ImGuiWindowFlags_NoDecoration|ImGuiWindowFlags_NoBackground))
    {

        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::BeginMenu("File"))
                {
                    if (ImGui::MenuItem("Open Image File.."))
                    {
                        flag_open_file_dialog_ = true;
                    }
                    ImGui::EndMenu();
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Edit"))
            {
                if (ImGui::MenuItem("Undo"))
                {
                    p_canvas_->undo();
                }
                if (ImGui::MenuItem("Redo"))
                {
                    p_canvas_->redo();
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Tools"))
            {
                if (ImGui::BeginMenu("Pen"))
                {
                    if (ImGui::MenuItem("Line"))
                    {
                        p_canvas_->set_line();
                        show_config_window = kPen;
                    }
                    if (ImGui::MenuItem("Rect"))
                    {
                        p_canvas_->set_rect();
                        show_config_window = kPen;
                    }
                    if (ImGui::MenuItem("Ellipse"))
                    {
                        p_canvas_->set_ellipse();
                        show_config_window = kPen;
                    }
                    if (ImGui::MenuItem("Polygon"))
                    {
                        p_canvas_->set_polygon();
                        show_config_window = kPen;
                    }
                    if (ImGui::MenuItem("FreeHand"))
                    {
                        p_canvas_->set_freehand();
                        show_config_window = kPen;
                    }
                    ImGui::EndMenu();
                }
                if (ImGui::MenuItem("Eraser"))
                {
                    show_config_window = kErase;
                }
                if (ImGui::MenuItem("Move tool"))
                {
                    show_config_window = kMove;
                }
                if (ImGui::MenuItem("Paint Bucket"))
                {
                    show_config_window = kPaint;
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("config"))
            {
                static ImVec4 colf = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
                ImGui::ColorEdit4(
                    "Color",
                    &colf.x,
                    ImGuiColorEditFlags_DisplayRGB |
                        ImGuiColorEditFlags_PickerHueBar |
                        ImGuiColorEditFlags_NoSidePreview);

                static float thickness = 3.0f;
                ImGui::SliderFloat(
                    "Circle segments override",
                    &thickness,
                    0.5,
                    10.0);

                p_canvas_->set_color(colf);
                p_canvas_->set_thickness(thickness);

                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }

        // Canvas component
        ImGui::Text("Press left mouse to draw, right mouse to stop.");
        // Set the canvas to fill the rest of the window
        const auto& canvas_min = ImGui::GetCursorScreenPos();
        const auto& canvas_size = ImGui::GetContentRegionAvail();
        p_canvas_->set_attributes(canvas_min, canvas_size);
        p_canvas_->draw();
    }
    // Context menu (under default mouse threshold)
    switch (show_config_window)
    {
        case USTC_CG::MiniDraw::kPen:
        {
            ImVec2 drag_delta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Right);
            if (drag_delta.x == 0.0f && drag_delta.y == 0.0f)
                ImGui::OpenPopupOnItemClick(
                    "context", ImGuiPopupFlags_MouseButtonRight);
            if (ImGui::BeginPopup("context"))
            {
                static ImVec4 colf = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
                ImGui::ColorEdit4(
                    "Color",
                    &colf.x,
                    ImGuiColorEditFlags_DisplayRGB |
                        ImGuiColorEditFlags_PickerHueBar |
                        ImGuiColorEditFlags_NoSidePreview);

                static float thickness = 3.0f;
                ImGui::SliderFloat(
                    "thick", &thickness, 0.5, 10.0);

                p_canvas_->set_color(colf);
                p_canvas_->set_thickness(thickness);

                ImGui::EndPopup();
            }
            break;
        }
        case USTC_CG::MiniDraw::kErase:
        {

        }
        // case USTC_CG::MiniDraw::kMove: break;
        // case USTC_CG::MiniDraw::kPaint: break;
        default: break;
    }
    ImGui::End();

}
}  // namespace USTC_CG