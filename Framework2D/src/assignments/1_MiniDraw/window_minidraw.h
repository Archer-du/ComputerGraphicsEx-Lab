#pragma once

#include <memory>

#include "view/window.h"
#include "view/comp_canvas.h"
#include "view/comp_image.h"

namespace USTC_CG
{
class MiniDraw : public Window
{
   public:
    explicit MiniDraw(const std::string& window_name);
    ~MiniDraw();

    enum ToolType
    {
        kNone,
        kPen,
        kErase,
        kMove,
        kPaint,
    };

    void draw();

   private:
    void draw_canvas();

    std::shared_ptr<Canvas> p_canvas_ = nullptr;
    std::shared_ptr<Image> p_image_ = nullptr;

    bool flag_show_canvas_view_ = true;
};
}  // namespace USTC_CG