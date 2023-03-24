#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "imgui-docking/imgui.h"

struct mesh {
    GLuint VBO = 0;
    GLuint VAO = 0;
    GLuint EBO = 0;
    GLuint vertexCount = 0;
};

struct position {
    double x = 0;
    double y = 0;
};

struct image_position {
    bool run_once = true;
    float scale = 1.0;

    position offset{};
    ImVec2 corner_pos;
    ImVec2 bottom_corner;
};

struct image_data {
    GLuint FRM_texture;
    uint8_t* FRM_data;
    GLuint PAL_texture;
    GLuint render_texture;
    GLuint framebuffer;
    int width;
    int height;

    image_position img_pos;
};

//FRM loading
bool init_framebuffer(struct image_data* img_data);
bool load_FRM_OpenGL(const char* file_name, image_data* img_data);
