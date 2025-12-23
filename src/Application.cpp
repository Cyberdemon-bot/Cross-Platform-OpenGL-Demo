#include "Application.h"
#include <iostream>

// --- VERTEX SHADER (MỚI) ---
// Thêm biến uniform "u_Model" kiểu Mat4 để nhân ma trận biến đổi
const char* vertexShaderSource = R"(
    #version 330 core
    layout (location = 0) in vec3 aPos;
    
    uniform mat4 u_Model; // Ma trận biến đổi (Vị trí, Xoay, Scale)

    void main() {
        // Phép nhân ma trận: Model * Position
        gl_Position = u_Model * vec4(aPos, 1.0);
    }
)";

const char* fragmentShaderSource = R"(
    #version 330 core
    out vec4 FragColor;
    uniform vec4 u_Color;
    void main() {
        FragColor = u_Color;
    }
)";

Application::Application() 
    : m_IsRunning(false), m_Window(nullptr), m_Context(nullptr), 
      m_VAO(0), m_VBO(0), m_EBO(0), m_ShaderProgram(0) {}

Application::~Application() { Clean(); }

bool Application::Init(const char* title, int width, int height) {
    // ... (Code Init giữ nguyên như bản Docking Viewport mình đã gửi trước đó) ...
    // Để cho gọn, bạn giữ nguyên phần Init cũ nhé. 
    // Chỉ đảm bảo là có gọi InitGraphics() ở cuối.
    
    // -- COPY LẠI ĐOẠN INIT CŨ VÀO ĐÂY --
    // Nếu lười copy lại, hãy dùng hàm Init full ở bài trước
    
    // (Giả sử bạn đã setup xong SDL, OpenGL, ImGui ở đây)
    m_Width = width; m_Height = height;
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) return false;
    
    // Setup OpenGL attributes...
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);

    m_Window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    m_Context = SDL_GL_CreateContext(m_Window);
    SDL_GL_MakeCurrent(m_Window, m_Context);
    gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress);

    // Setup ImGui...
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    ImGui_ImplSDL2_InitForOpenGL(m_Window, m_Context);
    ImGui_ImplOpenGL3_Init("#version 330");

    InitGraphics();
    m_IsRunning = true;
    return true;
}

void Application::InitGraphics() {
    CreateShaderProgram();

    // Hình vuông 4 đỉnh
    float vertices[] = {
         0.5f,  0.5f, 0.0f, // 0. Top Right
         0.5f, -0.5f, 0.0f, // 1. Bottom Right
        -0.5f, -0.5f, 0.0f, // 2. Bottom Left
        -0.5f,  0.5f, 0.0f  // 3. Top Left 
    };
    unsigned int indices[] = {  
        0, 1, 3,  // Tam giác 1
        1, 2, 3   // Tam giác 2
    };

    glGenVertexArrays(1, &m_VAO);
    glGenBuffers(1, &m_VBO);
    glGenBuffers(1, &m_EBO);

    glBindVertexArray(m_VAO);

    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0); 
}

void Application::CreateShaderProgram() {
    // (Code compile shader giữ nguyên, chỉ thay source ở trên đầu file)
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    m_ShaderProgram = glCreateProgram();
    glAttachShader(m_ShaderProgram, vertexShader);
    glAttachShader(m_ShaderProgram, fragmentShader);
    glLinkProgram(m_ShaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

void Application::Run() {
    while (m_IsRunning) {
        HandleEvents();
        Render();
    }
}

void Application::HandleEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        ImGui_ImplSDL2_ProcessEvent(&event);
        if (event.type == SDL_QUIT) m_IsRunning = false;
        if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) m_IsRunning = false;
        // Cập nhật lại kích thước OpenGL khi resize cửa sổ
        if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_RESIZED) {
             m_Width = event.window.data1;
             m_Height = event.window.data2;
             glViewport(0, 0, m_Width, m_Height);
        }
    }
}

void Application::Render() {
    // 1. ImGui Start
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    // 2. UI ĐIỀU KHIỂN
    ImGui::Begin("Control Panel");
    ImGui::Text("OpenGL 3.3 + ImGui + GLM");
    ImGui::Separator();
    
    ImGui::ColorEdit3("Shape Color", m_Color);
    ImGui::SliderFloat("Scale Size", &m_Scale, 0.1f, 2.0f);
    
    ImGui::Separator();
    ImGui::Checkbox("Auto Rotate", &m_AutoRotate);
    if (!m_AutoRotate) {
        ImGui::SliderFloat("Manual Angle", &m_RotationAngle, 0.0f, 360.0f);
    } else {
        ImGui::SliderFloat("Speed", &m_RotationSpeed, -5.0f, 5.0f);
        // Tự động cộng góc xoay
        m_RotationAngle += m_RotationSpeed; 
    }
    
    ImGui::Separator();
    if (ImGui::Checkbox("Wireframe Mode", &m_Wireframe)) {
        glPolygonMode(GL_FRONT_AND_BACK, m_Wireframe ? GL_LINE : GL_FILL);
    }
    ImGui::Separator();
    if (ImGui::Checkbox("Wireframe Mode", &m_Wireframe)) {
        glPolygonMode(GL_FRONT_AND_BACK, m_Wireframe ? GL_LINE : GL_FILL);
    }

    // --- TEST ASSIMP (MỚI THÊM VÀO ĐÂY) ---
    ImGui::Separator();
    ImGui::Text("System Check:");
    
    // Nút bấm kiểm tra
    if (ImGui::Button("Check Assimp Version")) {
        // 1. Khởi tạo thử Importer (Kiểm tra link class C++)
        Assimp::Importer importer;
        
        // 2. Lấy version (Kiểm tra link hàm C)
        int major = aiGetVersionMajor();
        int minor = aiGetVersionMinor();
        int revision = aiGetVersionRevision();

        // 3. In ra Terminal
        std::cout << "[SUCCESS] Assimp linked correctly!" << std::endl;
        std::cout << "   Version: " << major << "." << minor << "." << revision << std::endl;
        
        // (Optional) Hiển thị luôn lên tiêu đề cửa sổ cho oách
        std::string title = "Assimp v" + std::to_string(major) + "." + std::to_string(minor) + " works!";
        SDL_SetWindowTitle(m_Window, title.c_str());
    }
    ImGui::End();

    // 3. RENDER OPENGL
    glViewport(0, 0, m_Width, m_Height);
    glClearColor(0.1f, 0.1f, 0.15f, 1.0f); // Nền xanh tím than
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(m_ShaderProgram);

    // --- XỬ LÝ UNIFORM COLOR ---
    int colorLoc = glGetUniformLocation(m_ShaderProgram, "u_Color");
    glUniform4f(colorLoc, m_Color[0], m_Color[1], m_Color[2], 1.0f);

    // --- XỬ LÝ MA TRẬN (GLM) ---
    // a. Tạo ma trận đơn vị (Identity Matrix)
    glm::mat4 model = glm::mat4(1.0f); 
    
    // b. Áp dụng các phép biến đổi (Thứ tự: Scale -> Rotate -> Translate)
    // Lưu ý: Trong code viết ngược lại thứ tự mong muốn: Translate -> Rotate -> Scale
    // Ở đây ta xoay tại chỗ nên không cần Translate.
    
    // Xoay quanh trục Z (0,0,1)
    model = glm::rotate(model, glm::radians(m_RotationAngle), glm::vec3(0.0f, 0.0f, 1.0f)); 
    // Phóng to/Thu nhỏ
    model = glm::scale(model, glm::vec3(m_Scale, m_Scale, 1.0f)); 

    // c. Gửi ma trận xuống Shader
    int modelLoc = glGetUniformLocation(m_ShaderProgram, "u_Model");
    // tham số thứ 3 là GL_FALSE (không đảo chiều ma trận), tham số 4 là con trỏ dữ liệu
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model)); 

    // 4. DRAW
    glBindVertexArray(m_VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    // 5. ImGui Render & Viewports
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

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
    glDeleteVertexArrays(1, &m_VAO);
    glDeleteBuffers(1, &m_VBO);
    glDeleteBuffers(1, &m_EBO);
    glDeleteProgram(m_ShaderProgram);
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    SDL_DestroyWindow(m_Window);
    SDL_Quit();
}
