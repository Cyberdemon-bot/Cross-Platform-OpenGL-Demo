cmake_minimum_required(VERSION 3.10)
project(HazelClone LANGUAGES CXX C)

# =======================
# 0. BASIC SETUP
# =======================
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# =======================
# 1. DEFINITIONS (PATHS)
# =======================
set(VENDOR_DIR  ${PROJECT_SOURCE_DIR}/vendor)

# [SỬA ĐƯỜNG DẪN]: Viết thường folder glfw theo yêu cầu
set(GLFW_DIR    ${VENDOR_DIR}/glfw) 

set(GLAD_ROOT   ${VENDOR_DIR}/glad)
set(IMGUI_DIR   ${VENDOR_DIR}/imgui)
set(ASSIMP_ROOT ${VENDOR_DIR}/assimp)
set(STB_ROOT    ${VENDOR_DIR}/stb)

# =======================
# 2. BUILD GLFW FROM SOURCE (GLFW 3.4)
# =======================
# Tạo thư viện tĩnh tên là "glfw_lib"

# 2.1 Các file chung (Core)
set(GLFW_SOURCES
    "${GLFW_DIR}/include/GLFW/glfw3.h"
    "${GLFW_DIR}/include/GLFW/glfw3native.h"
    "${GLFW_DIR}/src/context.c"
    "${GLFW_DIR}/src/init.c"
    "${GLFW_DIR}/src/input.c"
    "${GLFW_DIR}/src/monitor.c"
    "${GLFW_DIR}/src/platform.c"  # Có trong GLFW 3.4
    "${GLFW_DIR}/src/vulkan.c"
    "${GLFW_DIR}/src/window.c"
    # Null Platform (Bắt buộc cho 3.4 làm fallback)
    "${GLFW_DIR}/src/null_init.c"
    "${GLFW_DIR}/src/null_joystick.c"
    "${GLFW_DIR}/src/null_monitor.c"
    "${GLFW_DIR}/src/null_window.c"
)

# 2.2 Cấu hình riêng cho macOS
if (APPLE)
    list(APPEND GLFW_SOURCES
        "${GLFW_DIR}/src/cocoa_init.m"
        "${GLFW_DIR}/src/cocoa_joystick.m"
        "${GLFW_DIR}/src/cocoa_monitor.m"
        "${GLFW_DIR}/src/cocoa_window.m"
        "${GLFW_DIR}/src/cocoa_time.c"
        "${GLFW_DIR}/src/nsgl_context.m"
        "${GLFW_DIR}/src/egl_context.c"
        "${GLFW_DIR}/src/osmesa_context.c"
        
        # [QUAN TRỌNG]: Các file POSIX bắt buộc cho GLFW 3.4 trên Mac
        "${GLFW_DIR}/src/posix_thread.c"
        "${GLFW_DIR}/src/posix_module.c"  # Fix lỗi __glfwPlatformLoadModule
        "${GLFW_DIR}/src/posix_poll.c"
    )
    # Định nghĩa macro thay vì dùng file config
    add_definitions(-D_GLFW_COCOA) 

elseif (WIN32)
    list(APPEND GLFW_SOURCES
        "${GLFW_DIR}/src/win32_init.c"
        "${GLFW_DIR}/src/win32_joystick.c"
        "${GLFW_DIR}/src/win32_monitor.c"
        "${GLFW_DIR}/src/win32_time.c"
        "${GLFW_DIR}/src/win32_thread.c"
        "${GLFW_DIR}/src/win32_window.c"
        "${GLFW_DIR}/src/win32_module.c"
        "${GLFW_DIR}/src/wgl_context.c"
        "${GLFW_DIR}/src/egl_context.c"
        "${GLFW_DIR}/src/osmesa_context.c"
    )
    add_definitions(-D_GLFW_WIN32 -D_CRT_SECURE_NO_WARNINGS)
endif()

# 2.3 Tạo target thư viện GLFW
add_library(glfw_lib STATIC ${GLFW_SOURCES})
target_include_directories(glfw_lib PUBLIC "${GLFW_DIR}/include")
target_include_directories(glfw_lib PRIVATE "${GLFW_DIR}/src")

# Link framework hệ thống Apple cho GLFW
if (APPLE)
    target_link_libraries(glfw_lib PRIVATE 
        "-framework Cocoa"
        "-framework IOKit"
        "-framework CoreFoundation"
        "-framework CoreVideo"
    )
endif()

# =======================
# 3. SETUP SOURCES (Main Game)
# =======================
# 3.1 Source Game
file(GLOB SOURCE_FILES CONFIGURE_DEPENDS src/*.cpp)

# 3.2 Source Glad
list(APPEND SOURCE_FILES "${GLAD_ROOT}/src/glad.c")

# 3.3 Source STB Image
list(APPEND SOURCE_FILES "${STB_ROOT}/stb_image/src/stb_image.cpp")

# 3.4 Source ImGui
# [QUAN TRỌNG]: Liệt kê rõ ràng để tránh lỗi thiếu file backend
set(IMGUI_SOURCES 
    "${IMGUI_DIR}/imgui.cpp"
    "${IMGUI_DIR}/imgui_demo.cpp"
    "${IMGUI_DIR}/imgui_draw.cpp"
    "${IMGUI_DIR}/imgui_tables.cpp"
    "${IMGUI_DIR}/imgui_widgets.cpp"
    # Backend Files (Fix lỗi Undefined symbols ImGui_ImplGlfw...)
    "${IMGUI_DIR}/backends/imgui_impl_glfw.cpp"
    "${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp"
)
list(APPEND SOURCE_FILES ${IMGUI_SOURCES})

# Tạo file thực thi
add_executable(main ${SOURCE_FILES})

# =======================
# 4. SETUP INCLUDE (GLOBAL)
# =======================
target_include_directories(main PRIVATE
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/vendor

    # GLAD
    ${GLAD_ROOT}/include

    # IMGUI
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends

    # ASSIMP
    ${ASSIMP_ROOT}/include
    
    # STB
    ${STB_ROOT}/stb_image/include

    # GLFW (Lấy từ target đã tạo)
    ${GLFW_DIR}/include
)

# =======================
# 5. SETUP LINKING (Main App)
# =======================

# Link thư viện GLFW source code
target_link_libraries(main PRIVATE glfw_lib)

if (APPLE)
    # ---------------------------------------------------------
    # macOS CONFIGURATION
    # ---------------------------------------------------------
    
    # Tìm file Assimp .dylib
    file(GLOB ASSIMP_MAC_LIB "${ASSIMP_ROOT}/lib/*.dylib")
    
    # Setup RPATH để chạy được file dylib mà không cần copy vào /usr/lib
    set_target_properties(main PROPERTIES BUILD_RPATH "${ASSIMP_ROOT}/lib;/Library/Frameworks")
    set_target_properties(main PROPERTIES INSTALL_RPATH "@executable_path/../vendor/assimp/lib;/Library/Frameworks")

    target_link_libraries(main PRIVATE 
        "-framework OpenGL"
        ${ASSIMP_MAC_LIB}
    )

elseif (WIN32)
    # ---------------------------------------------------------
    # Windows CONFIGURATION
    # ---------------------------------------------------------
    
    file(GLOB ASSIMP_WIN_LIB "${ASSIMP_ROOT}/lib/*.lib")
    file(GLOB ASSIMP_WIN_DLL "${ASSIMP_ROOT}/bin/*.dll")

    target_link_libraries(main PRIVATE 
        opengl32
        ${ASSIMP_WIN_LIB}
    )

    # Copy DLL Assimp ra folder chạy
    add_custom_command(TARGET main POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${ASSIMP_WIN_DLL}
            $<TARGET_FILE_DIR:main>
    )

else()
    message(FATAL_ERROR "Unsupported platform")
endif()
