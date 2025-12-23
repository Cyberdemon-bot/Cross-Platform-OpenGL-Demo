#pragma once

// --- INCLUDE GLM (MỚI) ---
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/version.h> // Để lấy số version

#include <glad/glad.h>
#include <SDL2/SDL.h>
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"

class Application {
public:
    Application();
    ~Application();

    bool Init(const char* title, int width, int height);
    void Run();

private:
    void InitGraphics();
    void CreateShaderProgram();
    void HandleEvents();
    void Update();
    void Render();
    void Clean();

private:
    bool m_IsRunning;
    SDL_Window* m_Window;
    SDL_GLContext m_Context;
    int m_Width;
    int m_Height;

    unsigned int m_VAO, m_VBO, m_EBO, m_ShaderProgram;

    // --- CÁC BIẾN ĐIỀU KHIỂN DEMO (MỚI) ---
    float m_Color[3] = { 0.2f, 0.8f, 1.0f }; // Màu (Mặc định xanh dương)
    float m_Scale = 0.5f;                    // Độ to nhỏ
    float m_RotationAngle = 0.0f;            // Góc xoay hiện tại
    float m_RotationSpeed = 1.0f;            // Tốc độ tự xoay
    bool m_AutoRotate = true;                // Bật/tắt tự xoay
    bool m_Wireframe = false;                // Chế độ khung dây
};
