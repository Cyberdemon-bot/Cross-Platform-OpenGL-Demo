#include "Application.h"
#include <iostream>

// --- SHADERS (OPENGL 4.1 CORE) ---
const char* vertexShaderSource = R"(
    #version 410 core
    layout (location = 0) in vec3 aPos;
    // layout (location = 1) in vec3 aNormal; 

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

Application::Application() 
    : m_IsRunning(false), m_Window(nullptr), m_Context(nullptr), m_ShaderProgram(0) {}

Application::~Application() { Clean(); }

bool Application::Init(const char* title, int width, int height) {
    m_Width = width; m_Height = height;

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
        std::cerr << "SDL Init Error: " << SDL_GetError() << std::endl;
        return false;
    }

    // --- SETUP OPENGL 4.1 CORE ---
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
    
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    // QUAN TRỌNG: SDL_WINDOW_ALLOW_HIGHDPI để hỗ trợ màn hình Retina sắc nét
    m_Window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    if (!m_Window) return false;

    m_Context = SDL_GL_CreateContext(m_Window);
    SDL_GL_MakeCurrent(m_Window, m_Context);
    
    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return false;
    }

    glEnable(GL_DEPTH_TEST); 

    // Setup ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; 
    
    ImGui::StyleColorsDark();

    ImGui_ImplSDL2_InitForOpenGL(m_Window, m_Context);
    ImGui_ImplOpenGL3_Init("#version 410");

    InitGraphics();
    LoadModelRaw("res/chess_pieces.glb"); // Nhớ check đường dẫn file

    m_IsRunning = true;
    return true;
}

void Application::Run() {
    while (m_IsRunning) {
        HandleEvents();
        Render();
    }
}

void Application::InitGraphics() {
    CreateShaderProgram();
}

void Application::CreateShaderProgram() {
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
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path, 
        aiProcess_Triangulate | 
        aiProcess_FlipUVs |
        aiProcess_PreTransformVertices 
    );

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

        if (AI_SUCCESS != aiGetMaterialColor(material, AI_MATKEY_BASE_COLOR, &color)) {
            aiGetMaterialColor(material, AI_MATKEY_COLOR_DIFFUSE, &color);
        }

        MeshPart part;
        part.indexOffset = currentIndexOffset;
        part.indexCount = mesh->mNumFaces * 3;
        part.color[0] = color.r;
        part.color[1] = color.g;
        part.color[2] = color.b;
        part.color[3] = color.a;
        
        m_MeshParts.push_back(part);

        for (unsigned int j = 0; j < mesh->mNumVertices; j++) {
            vertices.push_back(mesh->mVertices[j].x);
            vertices.push_back(mesh->mVertices[j].y);
            vertices.push_back(mesh->mVertices[j].z);
        }

        for (unsigned int j = 0; j < mesh->mNumFaces; j++) {
            aiFace face = mesh->mFaces[j];
            for (unsigned int k = 0; k < face.mNumIndices; k++) {
                indices.push_back(face.mIndices[k] + currentVertexOffset);
            }
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

void Application::HandleEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        ImGui_ImplSDL2_ProcessEvent(&event);
        if (event.type == SDL_QUIT) m_IsRunning = false;
        if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) m_IsRunning = false;
        
        // Lưu ý: Không cần set glViewport ở đây nữa, 
        // vì ta sẽ lấy kích thước thực (DrawableSize) trong hàm Render mỗi frame.
        if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_RESIZED) {
            m_Width = event.window.data1;
            m_Height = event.window.data2;
        }
    }
}

void Application::Render() {
    // 1. Setup Frame UI
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    // UI Controls
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

    // 2. XỬ LÝ VIEWPORT CHUẨN (Fix lỗi HighDPI/Retina)
    int displayW, displayH;
    // Lấy kích thước Pixel thực tế (trên Mac Retina nó sẽ gấp đôi m_Width/m_Height)
    SDL_GL_GetDrawableSize(m_Window, &displayW, &displayH);
    glViewport(0, 0, displayW, displayH);

    // Xóa màn hình
    glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (!m_MeshParts.empty()) {
        glUseProgram(m_ShaderProgram);

        // Tính Aspect Ratio dựa trên kích thước Pixel thực tế để hình không bị méo
        float aspectRatio = (float)displayW / (float)displayH;

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
            if (m_UseOverrideColor) {
                glUniform4f(colorLoc, m_OverrideColor[0], m_OverrideColor[1], m_OverrideColor[2], 1.0f);
            } else {
                glUniform4f(colorLoc, part.color[0], part.color[1], part.color[2], part.color[3]);
            }

            void* offsetPtr = (void*)(part.indexOffset * sizeof(unsigned int));
            glDrawElements(GL_TRIANGLES, part.indexCount, GL_UNSIGNED_INT, offsetPtr);
        }
        glBindVertexArray(0);
    }

    // 3. Render ImGui
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    
    // Xử lý Multi-viewport (kéo cửa sổ ImGui ra ngoài app)
    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        SDL_Window* backup_current_window = SDL_GL_GetCurrentWindow();
        SDL_GLContext backup_current_context = SDL_GL_GetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        SDL_GL_MakeCurrent(backup_current_window, backup_current_context);
    }

    SDL_GL_SwapWindow(m_Window);
}

void Application::Clean() {
    // (Giữ nguyên như cũ)
    glDeleteVertexArrays(1, &m_ModelVAO);
    glDeleteBuffers(1, &m_ModelVBO);
    glDeleteBuffers(1, &m_ModelEBO);
    glDeleteProgram(m_ShaderProgram);

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DeleteContext(m_Context);
    SDL_DestroyWindow(m_Window);
    SDL_Quit();
}
