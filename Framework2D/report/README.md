# HW1: report

---

> 姓名：杜朋澈
>
> ID: 68
>
> 学号：PB21050988

## 演示

- 绘制基本图形(Line, Rect, Ellipse)

  ![image-20240303224307420](C:\Users\Archer\AppData\Roaming\Typora\typora-user-images\image-20240303224307420.png)

  > 拖动左键开始绘制，松开左键停止绘制。

- 绘制多边形

  ![image-20240303224513043](C:\Users\Archer\AppData\Roaming\Typora\typora-user-images\image-20240303224513043.png)

  > 左键添加顶点，右键停止绘制。

- 自由绘制

  ![image-20240303224621837](C:\Users\Archer\AppData\Roaming\Typora\typora-user-images\image-20240303224621837.png)

  > 拖动左键开始绘制，松开左键停止绘制。

- 移动画布

  ![image-20240303224708306](C:\Users\Archer\AppData\Roaming\Typora\typora-user-images\image-20240303224708306.png)

  > 右键拖动即可

- 撤销与重做

  ![image-20240303224808927](C:\Users\Archer\AppData\Roaming\Typora\typora-user-images\image-20240303224808927.png)

  ![image-20240303224817460](C:\Users\Archer\AppData\Roaming\Typora\typora-user-images\image-20240303224817460.png)

  > `<Ctrl-Z> <Ctrl-Y>` 或通过 `Edit > Undo & Redo` 操作均可

- 调整画笔颜色和粗细

  ![image-20240303225957578](C:\Users\Archer\AppData\Roaming\Typora\typora-user-images\image-20240303225957578.png)

  ![image-20240303230031246](C:\Users\Archer\AppData\Roaming\Typora\typora-user-images\image-20240303230031246.png)

  > 鼠标中键弹出配置窗口

- 读取图片

  ![image-20240303225848197](C:\Users\Archer\AppData\Roaming\Typora\typora-user-images\image-20240303225848197.png)

## 功能设计

### 多边形与自由绘制

```c++
    draw_list->AddPolyline(
        temp.data(),
        temp.size(),
        ImGui::ColorConvertFloat4ToU32(color),
        true,
        thickness);
```

```c++
    draw_list->AddPolyline(
        temp.data(),
        temp.size(),
        ImGui::ColorConvertFloat4ToU32(color),
        false,
        thickness);
```

分别调取正确的内置API即可。

> 自由绘制时需要将draw flag设置为false，否则多边形会封口。

### 移动画布

修改主渲染循环：

```c++
void Canvas::draw()
{
    //get io...

    mouse_poll_event();

    draw_background();
    draw_shapes();
    draw_context();

    // some update...
}
```

将IO事件与具体逻辑解耦。其中，`poll_event`为:

```c++
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
```

订阅`right_drag_event`事件，完成如下逻辑即可：

```c++
    ImGuiIO& io = ImGui::GetIO();
    scrolling.x += io.MouseDelta.x;
    scrolling.y += io.MouseDelta.y;
	//逐一更新各个shape在view space的位置（基于canvas space的偏移）
    for (const auto& shape : shape_list_)
    {
        shape->updateOffset(io.MouseDelta.x, io.MouseDelta.y);
    }
```

### 撤销与重做

使用两个栈`undo_stack`和`redo_stack`来完成这个功能。

`undo_stack`和`redo_stack`在操作过程中的维护：

```c++
//在新的shape绘制完成时：
void Canvas::on_draw_stop()
{
    if (current_shape_)
    {
        shape_list_.push_back(current_shape_);
        //记录操作和涉及的shape（操作只有Insert，时间匆忙并没有实现delete，剪切粘贴等操作）
        undo_stack.push({ current_shape_, Insert });
        //注意在执行了新的操作后都必须将redo stack清空
        while (!redo_stack.empty())
        {
            redo_stack.pop();
        }
        current_shape_.reset();
    }
}
```

`undo` 和`redo`的具体操作：

```c++
void Canvas::undo()
{
    if (undo_stack.empty()) return;
    //弹出栈顶元素
    Operation op = undo_stack.top();
    undo_stack.pop();
    //根据操作类型修改shape（此处并未真正将shape销毁）
    switch (op.type)
    {
        case Insert:
            op.shape->enable = false;
            break;
        //......
        default: break;
    }
    redo_stack.push(op);
}

//同理
void Canvas::redo()
{
    if (redo_stack.empty()) return;
    Operation op = redo_stack.top();
    redo_stack.pop();
    switch (op.type)
    {
        case Insert:
            op.shape->enable = true;
        //......
            break;
    }
    undo_stack.push(op);
}
```

### 调整画笔颜色和粗细

调用如下API即可。

```c++
void Canvas::draw_context()
{
    ImGui::OpenPopupOnItemClick(
            "context", ImGuiPopupFlags_MouseButtonMiddle);
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
        ImGui::SliderFloat("thickness", &thickness, 0.5, 10.0);

        //在开启context菜单时不断更新颜色和粗细
        draw_color = colf;
        draw_thickness = thickness;

        ImGui::EndPopup();
    }
}
```

## UML

> 此处仅给出Target View的类图

![ClassDiagram](C:\Users\Archer\Desktop\ClassDiagram.png)

> 和助教和老师说声抱歉，最近准备春招面试太忙啦，实验做的很赶，也没时间详细写报告😔