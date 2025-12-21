#pragma once

#include <glad/glad.h>
#include <SDL2/SDL.h>
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"
#include <iostream>

class Application {
public:
    Application();
    ~Application();

    bool Init(const char* title, int width, int height);
    void Run();

private:
    void HandleEvents();
    void Update();
    void Render();
    void Clean();

    // --- MỚI: Hàm khởi tạo Tam giác & Shader ---
    void InitGraphics();
    void CreateShaderProgram();

private:
    bool m_IsRunning;
    SDL_Window* m_Window;
    SDL_GLContext m_Context;
    int m_Width;
    int m_Height;

    // --- MỚI: Các biến OpenGL ID ---
    // VAO: Vertex Array Object (Lưu cấu hình)
    // VBO: Vertex Buffer Object (Lưu dữ liệu thô)
    // ShaderProgram: ID của chương trình tô màu
    unsigned int m_VAO, m_VBO, m_ShaderProgram;
    float m_TriangleColor[3] = { 1.0f, 0.5f, 0.2f };
};