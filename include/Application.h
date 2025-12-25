#pragma once

// --- GLM MATHEMATICS ---
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// --- GRAPHICS & WINDOW ---
#include <glad/glad.h>
#include <GLFW/glfw3.h> // Thay SDL.h bằng GLFW

// --- GUI ---
#include "imgui.h"
#include "imgui_impl_glfw.h" // Thay imgui_impl_sdl2
#include "imgui_impl_opengl3.h"

// --- ASSIMP ---
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <vector>
#include <string>

struct MeshPart {
    unsigned int indexOffset;
    unsigned int indexCount;
    float color[4];
};

class Application {
public:
    Application();
    ~Application();

    bool Init(const char* title, int width, int height);
    void Run();

private:
    void InitGraphics();
    void CreateShaderProgram();
    void HandleEvents(); // Xử lý input GLFW
    void Render();
    void Clean();
    void LoadModelRaw(const char* path);

private:
    bool m_IsRunning;
    GLFWwindow* m_Window; // Thay SDL_Window*
    // GLFW không cần biến Context riêng lẻ như SDL, nó gắn liền với Window
    
    int m_Width;
    int m_Height;

    unsigned int m_ShaderProgram;

    // --- MODEL DATA ---
    unsigned int m_ModelVAO = 0;
    unsigned int m_ModelVBO = 0;
    unsigned int m_ModelEBO = 0;

    std::vector<MeshPart> m_MeshParts;

    // --- CONTROL ---
    float m_Scale = 1.0f;
    float m_RotationAngle = 0.0f;
    bool  m_AutoRotate = true;
    bool  m_Wireframe = false;
    bool  m_UseOverrideColor = false;
    float m_OverrideColor[3] = {1.0f, 1.0f, 1.0f};
};
