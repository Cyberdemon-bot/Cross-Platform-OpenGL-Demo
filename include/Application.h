#pragma once

// --- GLM MATHEMATICS ---
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// --- GRAPHICS & WINDOW ---
#include <glad/glad.h>
#include <SDL2/SDL.h>

// --- GUI ---
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"

// --- ASSIMP (MODEL LOADING) ---
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <vector>
#include <string>

// Struct chứa thông tin từng phần của model (để vẽ màu riêng biệt)
struct MeshPart {
    unsigned int indexOffset; // Vị trí bắt đầu trong EBO
    unsigned int indexCount;  // Số lượng đỉnh cần vẽ
    float color[4];           // Màu sắc (R, G, B, A)
};

class Application {
public:
    Application();
    ~Application();

    // Hàm khởi tạo cửa sổ và OpenGL
    bool Init(const char* title, int width, int height);
    
    // Hàm chạy vòng lặp game (Đã bị thiếu trước đó)
    void Run();

private:
    void InitGraphics();        // Setup Shader
    void CreateShaderProgram(); // Compile Shader
    void HandleEvents();        // Xử lý chuột/phím
    void Render();              // Vẽ hình
    void Clean();               // Dọn dẹp bộ nhớ

    // Hàm load model từ file (GLB, OBJ, FBX...)
    void LoadModelRaw(const char* path);

private:
    bool m_IsRunning;
    SDL_Window* m_Window;
    SDL_GLContext m_Context;
    int m_Width;
    int m_Height;

    unsigned int m_ShaderProgram;

    // --- MODEL DATA (VAO/VBO/EBO) ---
    unsigned int m_ModelVAO = 0;
    unsigned int m_ModelVBO = 0;
    unsigned int m_ModelEBO = 0;

    // Danh sách các phần của model
    std::vector<MeshPart> m_MeshParts;

    // --- BIẾN ĐIỀU KHIỂN (CONTROL) ---
    float m_Scale = 1.0f;
    float m_RotationAngle = 0.0f;
    bool  m_AutoRotate = true;
    bool  m_Wireframe = false;
    
    // Chế độ ghi đè màu (Override Color)
    bool  m_UseOverrideColor = false;
    float m_OverrideColor[3] = {1.0f, 1.0f, 1.0f};
};
