#include "Application.h"

// --- 1. CODE SHADER (Viết trực tiếp dạng chuỗi cho gọn) ---

// Vertex Shader: Xác định vị trí các đỉnh trên màn hình
const char* vertexShaderSource = R"(
    #version 330 core
    layout (location = 0) in vec3 aPos; // Nhận dữ liệu vị trí (Input)

    void main() {
        // Gán vị trí đỉnh (x, y, z, w)
        gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);
    }
)";

// Fragment Shader: Xác định màu sắc của từng điểm ảnh (pixel)
const char* fragmentShaderSource = R"(
    #version 330 core
    out vec4 FragColor;
    
    // Đây là biến Uniform: Cầu nối nhận dữ liệu từ C++
    uniform vec4 u_Color; 

    void main() {
        FragColor = u_Color; // Dùng màu được gửi vào
    }
)";

Application::Application() 
    : m_IsRunning(false), m_Window(nullptr), m_Context(nullptr), 
      m_VAO(0), m_VBO(0), m_ShaderProgram(0) {}

Application::~Application() {
    Clean();
}

bool Application::Init(const char* title, int width, int height) {
    m_Width = width;
    m_Height = height;

    // 1. KHỞI TẠO SDL SUB-SYSTEMS
    // Init Video (cho hiển thị) + Timer (để tính FPS) + GameController (nếu sau này dùng tay cầm)
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0) {
        std::cerr << "[Error] SDL Init failed: " << SDL_GetError() << std::endl;
        return false;
    }

    // 2. SETUP OPENGL ATTRIBUTES (Cấu hình trước khi tạo Window)
    const char* glsl_version = "#version 330";
    
    // Yêu cầu OpenGL 3.3 Core Profile (Chuẩn hiện đại)
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

    // Cấu hình bộ đệm màu (Color Buffer) và độ sâu (Depth Buffer - quan trọng cho 3D sau này)
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    // Riêng cho macOS cần cờ này để chạy được OpenGL 3.3+
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);

    // 3. TẠO WINDOW
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(
        SDL_WINDOW_OPENGL |        // Bắt buộc để dùng OpenGL
        SDL_WINDOW_RESIZABLE |     // Cho phép kéo dãn cửa sổ
        SDL_WINDOW_ALLOW_HIGHDPI   // Hỗ trợ màn hình Retina/4K sắc nét
    );
    
    m_Window = SDL_CreateWindow(
        title, 
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
        width, height, 
        window_flags
    );

    if (!m_Window) {
        std::cerr << "[Error] Create Window failed: " << SDL_GetError() << std::endl;
        return false;
    }

    // 4. TẠO CONTEXT OPENGL & LOAD GLAD
    m_Context = SDL_GL_CreateContext(m_Window);
    if (!m_Context) {
        std::cerr << "[Error] Create Context failed: " << SDL_GetError() << std::endl;
        return false;
    }
    
    SDL_GL_MakeCurrent(m_Window, m_Context);
    SDL_GL_SetSwapInterval(1); // Bật V-Sync (Chống xé hình, khóa FPS theo màn hình)

    // Load các hàm OpenGL qua Glad
    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
        std::cerr << "[Error] Failed to initialize Glad!" << std::endl;
        return false;
    }

    // In thông tin Card đồ họa ra màn hình console để kiểm tra
    std::cout << "--------------------------------------------------" << std::endl;
    std::cout << "GPU Vendor:   " << glGetString(GL_VENDOR) << std::endl;
    std::cout << "GPU Renderer: " << glGetString(GL_RENDERER) << std::endl;
    std::cout << "OpenGL Ver:   " << glGetString(GL_VERSION) << std::endl;
    std::cout << "--------------------------------------------------" << std::endl;

    // 5. SETUP IMGUI (FULL OPTION)
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    
    // --- BẬT CÁC TÍNH NĂNG CAO CẤP CỦA IMGUI ---
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Cho phép điều khiển bằng bàn phím
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;     // [Quan Trọng] Cho phép ghép các cửa sổ (Docking)
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;   // [Quan Trọng] Cho phép kéo cửa sổ ra màn hình ngoài (Multi-Viewport)
    
    // Chọn giao diện tối
    ImGui::StyleColorsDark();

    // Tinh chỉnh giao diện khi dùng Multi-Viewports (để các cửa sổ con trông đẹp hơn)
    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        style.WindowRounding = 0.0f;              // Cửa sổ tách ra sẽ vuông vức
        style.Colors[ImGuiCol_WindowBg].w = 1.0f; // Màu nền không bị trong suốt
    }

    // Init Backend (SDL2 + OpenGL3)
    ImGui_ImplSDL2_InitForOpenGL(m_Window, m_Context);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // 6. SETUP DỮ LIỆU ĐỒ HỌA RIÊNG CỦA GAME
    // (Load Shader, Tạo hình tam giác...)
    InitGraphics();

    m_IsRunning = true;
    return true;
}

// --- HÀM KHỞI TẠO DATA TAM GIÁC ---
void Application::InitGraphics() {
    // 1. Biên dịch Shader trước
    CreateShaderProgram();

    // 2. Định nghĩa dữ liệu 3 đỉnh (X, Y, Z)
    // OpenGL: X từ -1 đến 1, Y từ -1 đến 1
    float vertices[] = {
        -0.5f, -0.5f, 0.0f, // Trái dưới
         0.5f, -0.5f, 0.0f, // Phải dưới
         0.0f,  0.5f, 0.0f  // Đỉnh giữa
    };

    // 3. Tạo VAO và VBO
    glGenVertexArrays(1, &m_VAO);
    glGenBuffers(1, &m_VBO);

    // 4. BIND VAO trước! (Để ghi nhớ các cấu hình bên dưới)
    glBindVertexArray(m_VAO);

    // 5. Đẩy dữ liệu vào VBO
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // 6. Chỉ dẫn cho OpenGL cách đọc dữ liệu (Vertex Attribute Pointer)
    // "Này OpenGL, dữ liệu ở vị trí 0, gồm 3 số float (x,y,z), ko chuẩn hóa, bước nhảy là 3*float..."
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // 7. Unbind (cho an toàn, để ko lỡ tay sửa nhầm)
    glBindBuffer(GL_ARRAY_BUFFER, 0); 
    glBindVertexArray(0); 
}

// --- HÀM TẠO SHADER (Compile & Link) ---
void Application::CreateShaderProgram() {
    // A. Compile Vertex Shader
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    // (Nên thêm code check lỗi compile ở đây)

    // B. Compile Fragment Shader
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    // C. Link thành Program
    m_ShaderProgram = glCreateProgram();
    glAttachShader(m_ShaderProgram, vertexShader);
    glAttachShader(m_ShaderProgram, fragmentShader);
    glLinkProgram(m_ShaderProgram);

    // D. Dọn dẹp (Xóa shader lẻ sau khi đã link vào program)
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

void Application::Run() {
    while (m_IsRunning) {
        HandleEvents();
        Update();
        Render();
    }
}

void Application::HandleEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        ImGui_ImplSDL2_ProcessEvent(&event);
        if (event.type == SDL_QUIT) m_IsRunning = false;
        if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) m_IsRunning = false;
    }
}

void Application::Update() {
    // Logic game...
}

void Application::Render() {
    // 1. Setup Frame (Giữ nguyên)
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
    
    // ... (Code vẽ UI của bạn: Begin, End, ColorEdit...) ...
    // Ví dụ:
    ImGui::Begin("Triangle Settings");
    ImGui::Text("Keo tui ra khoi cua so chinh di!"); // Thử thách người dùng
    ImGui::ColorEdit3("Color", m_TriangleColor); 
    ImGui::End();

    // 2. Render Scene Chính (Giữ nguyên)
    ImGui::Render();
    glViewport(0, 0, m_Width, m_Height);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(m_ShaderProgram);
    int colorLocation = glGetUniformLocation(m_ShaderProgram, "u_Color");
    glUniform4f(colorLocation, m_TriangleColor[0], m_TriangleColor[1], m_TriangleColor[2], 1.0f);
    glBindVertexArray(m_VAO);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    // Vẽ data của ImGui lên cửa sổ chính
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // --- 3. XỬ LÝ VIEWPORTS (MỚI - QUAN TRỌNG) ---
    ImGuiIO& io = ImGui::GetIO();
    // Nếu tính năng Viewports đang bật
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        // Backup context hiện tại (của cửa sổ chính)
        SDL_Window* backup_current_window = SDL_GL_GetCurrentWindow();
        SDL_GLContext backup_current_context = SDL_GL_GetCurrentContext();
        
        // ImGui tự xử lý việc tạo window mới và render vào đó
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        
        // Restore lại context cho cửa sổ chính (để vòng lặp sau không bị lỗi)
        SDL_GL_MakeCurrent(backup_current_window, backup_current_context);
    }
    // ---------------------------------------------

    // 4. Swap Buffer (Giữ nguyên)
    SDL_GL_SwapWindow(m_Window);
}

void Application::Clean() {
    // Xóa tài nguyên GPU
    glDeleteVertexArrays(1, &m_VAO);
    glDeleteBuffers(1, &m_VBO);
    glDeleteProgram(m_ShaderProgram);

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    SDL_DestroyWindow(m_Window);
    SDL_Quit();
}