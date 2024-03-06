#include "window_minidraw.h"
#include <ImGuiFileDialog.h>

#include <iostream>

namespace USTC_CG
{
MiniDraw::MiniDraw(const std::string& window_name) : Window(window_name)
{
    p_canvas_ =
        std::make_shared<Canvas>("Cmpt.Canvas", ImVec4(1.0f, 0.0f, 0.0f, 1.0f), 2);
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
                if (ImGui::MenuItem("Hand"))
                {
                    p_canvas_->set_default();
                }
                if (ImGui::MenuItem("Line"))
                {
                    p_canvas_->set_line();
                }
                if (ImGui::MenuItem("Rect"))
                {
                    p_canvas_->set_rect();
                }
                if (ImGui::MenuItem("Ellipse"))
                {
                    p_canvas_->set_ellipse();
                }
                if (ImGui::MenuItem("Polygon"))
                {
                    p_canvas_->set_polygon();
                }
                if (ImGui::MenuItem("FreeHand"))
                {
                    p_canvas_->set_freehand();
                }
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }

        // Set the canvas to fill the rest of the window
        const auto& canvas_min = ImGui::GetCursorScreenPos();
        const auto& canvas_size = ImGui::GetContentRegionAvail();
        p_canvas_->set_attributes(canvas_min, canvas_size);
        p_canvas_->draw();

        if (p_image_)
        {
            const auto& image_size = p_image_->get_image_size();
            // Center the image in the window
            ImVec2 pos = ImVec2(
                canvas_min.x + canvas_size.x / 2 - image_size.x / 2,
                canvas_min.y + canvas_size.y / 2 - image_size.y / 2);
            p_image_->set_position(pos);
            p_image_->draw();
        }
    }

    ImGui::End();

}
}  // namespace USTC_CG