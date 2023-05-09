#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdlib.h>

#include "imgui-docking/imgui.h"

struct rectangle {
    int x1;
    int y1;

    int x2;
    int y2;
};

struct mesh {
    GLuint VBO = 0;
    GLuint VAO = 0;
    GLuint EBO = 0;
    GLuint vertexCount = 0;
};

#pragma pack(push, 1)
struct FRM_Header {
    uint32_t version;                           // 0x0000
    uint16_t FPS = 0;                           // 0x0004
    uint16_t Action_Frame = 0;                  // 0x0006
    uint16_t Frames_Per_Orient;                 // 0x0008
    int16_t  Shift_Orient_x[6];                 // 0x000A
    int16_t  Shift_Orient_y[6];                 // 0x0016
    uint32_t Frame_0_Offset[6];                 // 0x0022
    uint32_t Frame_Area;                        // 0x003A
};

struct FRM_Frame_Info {
    uint16_t Frame_Width;                       // 0x003E
    uint16_t Frame_Height;                      // 0x0040
    uint32_t Frame_Size;                        // 0x0042
    int16_t  Shift_Offset_x;                    // 0x0046
    int16_t  Shift_Offset_y;                    // 0x0048
    uint8_t  frame_start[];
};
#pragma pack(pop)

struct FRM_Frame {
    //TODO: remove unnecesary info?
    int frame_number = 0;
    int orientation  = 0;
    FRM_Frame_Info* frame_info;
    rectangle bounding_box = {};
};

struct image_data {
    //TODO: convert FRM_Info to a pointer and adjust save function to handle
    FRM_Header* FRM_Info;
    FRM_Frame* Frame;
    rectangle FRM_bounding_box[6];

    uint8_t* FRM_data = NULL;
    uint8_t* MSK_data = NULL;

    GLuint FRM_texture;
    GLuint MSK_texture;
    GLuint PAL_texture;
    GLuint render_texture;
    GLuint framebuffer;
    int width;
    int height;
    int display_frame_num;
    int display_orient_num;
    int playback_speed;

    float scale = 1.0;

    ImVec2 offset{};
};

//FRM loading
bool init_framebuffer(struct image_data* img_data);
bool load_FRM_OpenGL(const char* file_name, image_data* img_data);
