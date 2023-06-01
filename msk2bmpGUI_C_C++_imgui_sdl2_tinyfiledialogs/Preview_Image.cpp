#include "imgui-docking/imgui_internal.h"
#include "display_FRM_OpenGL.h"
#include "Preview_Image.h"
#include "Zoom_Pan.h"


void Preview_FRM_Image(variables* My_Variables, struct image_data* img_data, bool show_stats)
{
    ImVec2 top_of_window = ImGui::GetCursorPos();

    //handle zoom and panning for the image, plus update image position every frame
    zoom_pan(img_data, My_Variables->new_mouse_pos, My_Variables->mouse_delta);

    //new openGL version of pallete cycling
    //redraws FRM to framebuffer every time the palette update timer is true or animates
    shader_info* shaders = &My_Variables->shaders;
    animate_FRM_to_framebuff(shaders->palette,
                             shaders->render_FRM_shader,
                             shaders->giant_triangle,
                             img_data,
                             My_Variables->CurrentTime,
                             My_Variables->Palette_Update);

    //draw_FRM_to_framebuffer(shaders->palette,
    //    shaders->render_FRM_shader,
    //    &shaders->giant_triangle,
    //    img_data);

    //handle frame display by orientation and number
    int orient  = img_data->display_orient_num;
    int frame   = img_data->display_frame_num;
    int max_frm = img_data->FRM_dir[orient].num_frames;

    float scale = img_data->scale;
    int width   = img_data->FRM_bounding_box[orient].x2 - img_data->FRM_bounding_box[orient].x1;
    int height  = img_data->FRM_bounding_box[orient].y2 - img_data->FRM_bounding_box[orient].y1;
    ImVec2 uv_min = My_Variables->uv_min;      // (0.0f,0.0f)
    ImVec2 uv_max = My_Variables->uv_max;      // (1.0f,1.0f)
    ImVec2 size = ImVec2((float)(width * scale), (float)(height * scale));


    ImGuiWindow* window = ImGui::GetCurrentWindow();
    //image I'm trying to pan and zoom with
    window->DrawList->AddImage(
        (ImTextureID)img_data->render_texture,
        top_corner(img_data), bottom_corner(size, top_corner(img_data)),
        uv_min, uv_max,
        ImGui::GetColorU32(My_Variables->tint_col));

    //TODO: need to figure out how I'm going to handle scrolling on large images
    ImGui::Dummy(size);

    //show FRM stats over FRM
    ImGui::SetCursorPos(top_of_window);
    if (show_stats) {
        show_image_stats_FRM(img_data, My_Variables->Font);
    }

}

void Preview_Image(variables* My_Variables, struct image_data* img_data, bool show_stats)
{
    ImVec2 top_of_window = ImGui::GetCursorPos();

    //handle zoom and panning for the image, plus update image position every frame
    zoom_pan(img_data, My_Variables->new_mouse_pos, My_Variables->mouse_delta);

    shader_info* shaders = &My_Variables->shaders;
    animate_OTHER_to_framebuff(My_Variables->shaders.render_OTHER_shader,
        &My_Variables->shaders.giant_triangle,
        img_data,
        My_Variables->CurrentTime);

    //handle frame display by orientation and number
    float scale = img_data->scale;
    int width  = img_data->width;
    int height = img_data->height;
    ImVec2 uv_min = My_Variables->uv_min;      // (0.0f,0.0f)
    ImVec2 uv_max = My_Variables->uv_max;      // (1.0f,1.0f)
    ImVec2 size = ImVec2((float)(width * scale), (float)(height * scale));

    ImGuiWindow* window = ImGui::GetCurrentWindow();
    //image I'm trying to pan and zoom with
    window->DrawList->AddImage(
        (ImTextureID)img_data->render_texture,
        top_corner(img_data), bottom_corner(size, top_corner(img_data)),
        uv_min, uv_max,
        ImGui::GetColorU32(My_Variables->tint_col));

    //TODO: need to figure out how I'm going to handle scrolling on large images
    ImGui::Dummy(size);

    //show image stats over image
    ImGui::SetCursorPos(top_of_window);
    if (show_stats) {
        show_image_stats_ANM(img_data, My_Variables->Font);
    }

}

void draw_red_squares(LF* F_Prop, bool wrong_size)
{
    // Draw red boxes to indicate where the tiles will be cut from
    float scale = F_Prop->img_data.scale;
    if (wrong_size) {
        ImDrawList *Draw_List = ImGui::GetWindowDrawList();
        ImVec2 Origin;
        Origin.x = F_Prop->img_data.offset.x + ImGui::GetItemRectMin().x;
        Origin.y = F_Prop->img_data.offset.y + ImGui::GetItemRectMin().y;

        ImVec2 Top_Left;
        ImVec2 Bottom_Right = { 0, 0 };
        int max_box_x = F_Prop->IMG_Surface->w / 350;
        int max_box_y = F_Prop->IMG_Surface->h / 300;

        for (int i = 0; i < max_box_x; i++)
        {
            for (int j = 0; j < max_box_y; j++)
            {
                Top_Left.x = Origin.x + (i * 350)*scale;
                Top_Left.y = Origin.y + (j * 300)*scale;
                Bottom_Right = { (float)(Top_Left.x + 350 * scale), (float)(Top_Left.y + 300 * scale) };
                Draw_List->AddRect(Top_Left, Bottom_Right, 0xff0000ff, 0, 0, 5.0f);
            }
        }
    }
}

void show_image_stats_FRM(image_data* img_data, ImFont* font)
{
    int num, dir, max;
    num =  img_data->display_frame_num;
    dir = (img_data->FRM_hdr->Frame_0_Offset[1] > 0) ? img_data->display_orient_num : 0;
    char buff[256];

    ImGui::PushFont(font);
    snprintf(buff, 256, "framerate: %d",        img_data->FRM_hdr->FPS);
    ImGui::Text(buff);

    snprintf(buff, 256, "orient_shift_x: %d",   img_data->FRM_hdr->Shift_Orient_x[dir]);
    ImGui::Text(buff);
    snprintf(buff, 256, "orient_shift_y: %d",   img_data->FRM_hdr->Shift_Orient_y[dir]);
    ImGui::Text(buff);

    snprintf(buff, 256, "bounding_x1: %d\t",    img_data->FRM_dir[dir].bounding_box[num].x1);
    ImGui::Text(buff);
    ImGui::SameLine();
    snprintf(buff, 256, "width: %d\t",          img_data->FRM_dir[dir].frame_data[num]->Frame_Width);
    ImGui::Text(buff);
    ImGui::SameLine();
    snprintf(buff, 256, "x_offset: %d",         img_data->FRM_dir[dir].frame_data[num]->Shift_Offset_x);
    ImGui::Text(buff);
    snprintf(buff, 256, "bounding_x2: %d",      img_data->FRM_dir[dir].bounding_box[num].x2);
    ImGui::Text(buff);

    snprintf(buff, 256, "bounding_y1: %d\t",    img_data->FRM_dir[dir].bounding_box[num].y1);
    ImGui::Text(buff);
    ImGui::SameLine();
    snprintf(buff, 256, "height: %d\t",         img_data->FRM_dir[dir].frame_data[num]->Frame_Height);
    ImGui::Text(buff);
    ImGui::SameLine();
    snprintf(buff, 256, "y_offset: %d",         img_data->FRM_dir[dir].frame_data[num]->Shift_Offset_y);
    ImGui::Text(buff);
    snprintf(buff, 256, "bounding_y2: %d",      img_data->FRM_dir[dir].bounding_box[num].y2);
    ImGui::Text(buff);

    snprintf(buff, 256, "FRM_bounding_x1: %d",  img_data->FRM_bounding_box[dir].x1);
    ImGui::Text(buff);
    snprintf(buff, 256, "FRM_bounding_x2: %d",  img_data->FRM_bounding_box[dir].x2);
    ImGui::Text(buff);
    snprintf(buff, 256, "FRM_bounding_y1: %d",  img_data->FRM_bounding_box[dir].y1);
    ImGui::Text(buff);
    snprintf(buff, 256, "FRM_bounding_y2: %d",  img_data->FRM_bounding_box[dir].y2);
    ImGui::Text(buff);
    ImGui::PopFont();
}

void show_image_stats_ANM(image_data* img_data, ImFont* font)
{
    int num, dir, max;
    num = img_data->display_frame_num;
    dir = img_data->display_orient_num;
    char buff[256];

    ImGui::PushFont(font);
    //snprintf(buff, 256, "framerate: %d", img_data->ANM_hdr->FPS);
    //ImGui::Text(buff);

    //snprintf(buff, 256, "orient_shift_x: %d", img_data->ANM_hdr->Shift_Orient_x[r]);
    //ImGui::Text(buff);
    //snprintf(buff, 256, "orient_shift_y: %d", img_data->ANM_hdr->Shift_Orient_y[r]);
    //ImGui::Text(buff);

    snprintf(buff, 256, "bounding_x1: %d\t",    img_data->ANM_dir[dir].bounding_box.x1);
    ImGui::Text(buff);
    ImGui::SameLine();
    snprintf(buff, 256, "width: %d\t",          img_data->ANM_dir[dir].frame_data[num].Frame_Width);
    ImGui::Text(buff);
    ImGui::SameLine();
    snprintf(buff, 256, "x_offset: %d",         img_data->ANM_dir[dir].frame_data[num].Shift_Offset_x);
    ImGui::Text(buff);
    snprintf(buff, 256, "bounding_x2: %d",      img_data->ANM_dir[dir].bounding_box.x2);
    ImGui::Text(buff);

    snprintf(buff, 256, "bounding_y1: %d\t",    img_data->ANM_dir[dir].bounding_box.y1);
    ImGui::Text(buff);
    ImGui::SameLine();
    snprintf(buff, 256, "height: %d\t",         img_data->ANM_dir[dir].frame_data[num].Frame_Height);
    ImGui::Text(buff);
    ImGui::SameLine();
    snprintf(buff, 256, "y_offset: %d",         img_data->ANM_dir[dir].frame_data[num].Shift_Offset_y);
    ImGui::Text(buff);
    snprintf(buff, 256, "bounding_y2: %d",      img_data->ANM_dir[dir].bounding_box.y2);
    ImGui::Text(buff);

    snprintf(buff, 256, "ANM_bounding_x1: %d",  img_data->ANM_bounding_box[dir].x1);
    ImGui::Text(buff);
    snprintf(buff, 256, "ANM_bounding_x2: %d",  img_data->ANM_bounding_box[dir].x2);
    ImGui::Text(buff);
    snprintf(buff, 256, "ANM_bounding_y1: %d",  img_data->ANM_bounding_box[dir].y1);
    ImGui::Text(buff);
    snprintf(buff, 256, "ANM_bounding_y2: %d",  img_data->ANM_bounding_box[dir].y2);
    ImGui::Text(buff);
    ImGui::PopFont();
}