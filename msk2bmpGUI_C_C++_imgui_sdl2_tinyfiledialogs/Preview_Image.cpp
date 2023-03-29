#include "Preview_Image.h"


void Preview_Image(variables* My_Variables, struct image_data* img_data)
{
    float scale = img_data->scale;
    int width   = img_data->width;
    int height  = img_data->height;
    ImVec2 uv_min = My_Variables->uv_min;      // (0.0f,0.0f)
    ImVec2 uv_max = My_Variables->uv_max;      // (1.0f,1.0f)

    //handle zoom and panning for the image, plus update image position every frame
    //ImVec2 corner_pos;
    //ImVec2 bottom_corner;
    zoom_pan(img_data, My_Variables->new_mouse_pos, My_Variables->mouse_delta);

    ImVec2 size = ImVec2((float)(width * scale), (float)(height * scale));
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    //image I'm trying to pan and zoom with
    window->DrawList->AddImage(
        (ImTextureID)img_data->render_texture,
        top_corner(img_data), bottom_corner(size, top_corner(img_data)),
        //corner_pos, bottom_corner,
        uv_min, uv_max,
        ImGui::GetColorU32(My_Variables->tint_col));


    //TODO: need to figure out how I'm going to handle scrolling on large images
    ImGui::Dummy(size);

}