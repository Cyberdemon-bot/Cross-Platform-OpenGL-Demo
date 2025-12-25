#include "Application.h"
#include <iostream>

// --- SHADERS (GIỮ NGUYÊN) ---
const char* vertexShaderSource = R"(
    #version 410 core
    layout (location = 0) in vec3 aPos;
    uniform mat4 u_Model;
    uniform mat4 u_View;
    uniform mat4 u_Projection;
    void main() {
        gl_Position = u_Projection * u_View * u_Model * vec4(aPos, 1.0);
    }
)";

const char* fragmentShaderSource = R"(
    #version 410 core
    out vec4 FragColor;
    uniform vec4 u_Color; 
    void main() {
        FragColor = u_Color;
    }
)";

// Callback báo lỗi của GLFW
void GLFWErrorCallback(int error, const char* description) {
    std::cerr << "GLFW Error (" << error << "): " << description << std::endl;
}

Application::Application() 
    : m_IsRunning(false), m_Window(nullptr), m_ShaderProgram(0) {}

Application::~Application() { Clean(); }

bool Application::Init(const char* title, int width, int height) {
    m_Width = width; m_Height = height;

    // 1. Init GLFW
    glfwSetErrorCallback(GLFWErrorCallback);
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return false;
    }

    // 2. Setup OpenGL Version (4.1 Core for Mac)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // Bắt buộc cho Mac
    glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GLFW_TRUE); // Hỗ trợ Retina

    // 3. Create Window
    m_Window = glfwCreateWindow(width, height, title, NULL, NULL);
    if (!m_Window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(m_Window);
    // Tắt V-Sync (0) hoặc Bật (1)
    glfwSwapInterval(1); 

    // 4. Init GLAD
    // Lưu ý: cast sang GLADloadproc
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return false;
    }

    glEnable(GL_DEPTH_TEST); 

    // 5. Setup ImGui (Backend GLFW)
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; 
    // io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // Nếu muốn Multi-viewport

    ImGui::StyleColorsDark();

    // Init cho GLFW
    ImGui_ImplGlfw_InitForOpenGL(m_Window, true); // true = install callbacks
    ImGui_ImplOpenGL3_Init("#version 410");

    InitGraphics();
    LoadModelRaw("res/chess_pieces.glb");

    m_IsRunning = true;
    return true;
}

void Application::Run() {
    // GLFW dùng double (giây) thay vì Uint32 (ms)
    double lastTime = glfwGetTime(); 
    int frameCount = 0;
    std::string originalTitle = "OpenGL 4.1 Viewer (GLFW)"; 

    while (!glfwWindowShouldClose(m_Window) && m_IsRunning) {
        HandleEvents();
        Render();

        // --- TÍNH FPS ---
        double currentTime = glfwGetTime();
        frameCount++;
        
        // Nếu qua 1.0 giây
        if (currentTime - lastTime >= 1.0) {
            std::string title = originalTitle + " - FPS: " + std::to_string(frameCount);
            glfwSetWindowTitle(m_Window, title.c_str());
            frameCount = 0;
            lastTime = currentTime;
        }
    }
}

void Application::InitGraphics() { CreateShaderProgram(); }

// --- CreateShaderProgram và LoadModelRaw GIỮ NGUYÊN KHÔNG ĐỔI ---
void Application::CreateShaderProgram() {
    // (Copy y hệt code cũ, không thay đổi logic OpenGL)
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    
    int success; char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if(!success) {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cerr << "Vertex Shader Error: " << infoLog << std::endl;
    }

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if(!success) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cerr << "Fragment Shader Error: " << infoLog << std::endl;
    }

    m_ShaderProgram = glCreateProgram();
    glAttachShader(m_ShaderProgram, vertexShader);
    glAttachShader(m_ShaderProgram, fragmentShader);
    glLinkProgram(m_ShaderProgram);
    
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

void Application::LoadModelRaw(const char* path) {
    // (Copy y hệt code cũ)
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path, 
        aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_PreTransformVertices);

    if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cerr << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
        return;
    }

    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    m_MeshParts.clear(); 
    
    unsigned int currentVertexOffset = 0;
    unsigned int currentIndexOffset = 0;

    for (unsigned int i = 0; i < scene->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[i];
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
        aiColor4D color(0.8f, 0.8f, 0.8f, 1.0f);

        if (AI_SUCCESS != aiGetMaterialColor(material, AI_MATKEY_BASE_COLOR, &color))
            aiGetMaterialColor(material, AI_MATKEY_COLOR_DIFFUSE, &color);

        MeshPart part;
        part.indexOffset = currentIndexOffset;
        part.indexCount = mesh->mNumFaces * 3;
        part.color[0] = color.r; part.color[1] = color.g; part.color[2] = color.b; part.color[3] = color.a;
        m_MeshParts.push_back(part);

        for (unsigned int j = 0; j < mesh->mNumVertices; j++) {
            vertices.push_back(mesh->mVertices[j].x);
            vertices.push_back(mesh->mVertices[j].y);
            vertices.push_back(mesh->mVertices[j].z);
        }
        for (unsigned int j = 0; j < mesh->mNumFaces; j++) {
            aiFace face = mesh->mFaces[j];
            for (unsigned int k = 0; k < face.mNumIndices; k++)
                indices.push_back(face.mIndices[k] + currentVertexOffset);
        }
        currentVertexOffset += mesh->mNumVertices;
        currentIndexOffset += mesh->mNumFaces * 3;
    }

    if (m_ModelVAO == 0) glGenVertexArrays(1, &m_ModelVAO);
    if (m_ModelVBO == 0) glGenBuffers(1, &m_ModelVBO);
    if (m_ModelEBO == 0) glGenBuffers(1, &m_ModelEBO);

    glBindVertexArray(m_ModelVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_ModelVBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ModelEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
}
// -------------------------------------------------------------

void Application::HandleEvents() {
    // GLFW đơn giản hơn SDL, chỉ cần poll
    glfwPollEvents();

    // Check nút Escape
    if (glfwGetKey(m_Window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        m_IsRunning = false;
        glfwSetWindowShouldClose(m_Window, true);
    }

    // Không cần xử lý Resize thủ công ở đây, 
    // ta lấy FramebufferSize trực tiếp trong hàm Render
}

void Application::Render() {
    // 1. Setup Frame UI (GLFW backend)
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame(); // Thay SDL
    ImGui::NewFrame();

    // UI Controls (Giữ nguyên)
    ImGui::Begin("OpenGL 4.1 Viewer");
    ImGui::DragFloat("Scale", &m_Scale, 0.01f, 0.01f, 10.0f);
    ImGui::Checkbox("Auto Rotate", &m_AutoRotate);
    if (!m_AutoRotate) ImGui::SliderFloat("Rotation", &m_RotationAngle, 0.0f, 360.0f);
    else m_RotationAngle += 0.5f;
    ImGui::Checkbox("Wireframe", &m_Wireframe);
    ImGui::Checkbox("Override Color", &m_UseOverrideColor);
    if (m_UseOverrideColor) ImGui::ColorEdit3("Color", m_OverrideColor);
    
    static char pathBuf[128] = "res/chess_pieces.glb";
    ImGui::InputText("Path", pathBuf, IM_ARRAYSIZE(pathBuf));
    if (ImGui::Button("Load Model")) LoadModelRaw(pathBuf);
    ImGui::End();

    // 2. Viewport & Retina Support
    int displayW, displayH;
    // GLFW thay SDL_GL_GetDrawableSize
    glfwGetFramebufferSize(m_Window, &displayW, &displayH);
    glViewport(0, 0, displayW, displayH);

    glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (!m_MeshParts.empty()) {
        glUseProgram(m_ShaderProgram);
        
        float aspectRatio = (float)displayW / (float)displayH;
        // Nếu minimize window, aspect ratio có thể NaN, cần check
        if (displayH == 0) aspectRatio = 1.0f;

        glm::mat4 projection = glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 1000.0f);
        glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -2.0f, -8.0f)); 
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::rotate(model, glm::radians(m_RotationAngle), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(m_Scale));

        glUniformMatrix4fv(glGetUniformLocation(m_ShaderProgram, "u_Projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(glGetUniformLocation(m_ShaderProgram, "u_View"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(m_ShaderProgram, "u_Model"), 1, GL_FALSE, glm::value_ptr(model));

        glBindVertexArray(m_ModelVAO);
        glPolygonMode(GL_FRONT_AND_BACK, m_Wireframe ? GL_LINE : GL_FILL);

        int colorLoc = glGetUniformLocation(m_ShaderProgram, "u_Color");
        for (const auto& part : m_MeshParts) {
            if (m_UseOverrideColor)
                glUniform4f(colorLoc, m_OverrideColor[0], m_OverrideColor[1], m_OverrideColor[2], 1.0f);
            else
                glUniform4f(colorLoc, part.color[0], part.color[1], part.color[2], part.color[3]);

            void* offsetPtr = (void*)(part.indexOffset * sizeof(unsigned int));
            glDrawElements(GL_TRIANGLES, part.indexCount, GL_UNSIGNED_INT, offsetPtr);
        }
        glBindVertexArray(0);
    }

    // 3. Render ImGui
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    
    // Xử lý Viewports (Nếu bật)
    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        GLFWwindow* backup_current_context = glfwGetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        glfwMakeContextCurrent(backup_current_context);
    }

    // Swap buffers (GLFW)
    glfwSwapBuffers(m_Window);
}

void Application::Clean() {
    glDeleteVertexArrays(1, &m_ModelVAO);
    glDeleteBuffers(1, &m_ModelVBO);
    glDeleteBuffers(1, &m_ModelEBO);
    glDeleteProgram(m_ShaderProgram);

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown(); // Shutdown GLFW backend
    ImGui::DestroyContext();

    glfwDestroyWindow(m_Window);
    glfwTerminate(); // Cleanup GLFW
}
