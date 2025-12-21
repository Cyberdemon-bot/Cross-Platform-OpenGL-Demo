cmake_minimum_required(VERSION 3.10)
project(MySDLGame LANGUAGES CXX C) # C cho glad, CXX cho phần còn lại

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# =======================
# 0. DEFINITIONS
# =======================
# Định nghĩa các đường dẫn gốc
set(VENDOR_DIR ${PROJECT_SOURCE_DIR}/vendor)
set(SDL_ROOT   ${VENDOR_DIR}/SDL2)
set(GLAD_ROOT  ${VENDOR_DIR}/glad)
set(IMGUI_DIR  ${VENDOR_DIR}/imgui) # <--- [MỚI] Định nghĩa đường dẫn ImGui

# =======================
# 1. SETUP SOURCES
# =======================
# 1.1 Lấy source code của GAME
file(GLOB SOURCE_FILES CONFIGURE_DEPENDS src/*.cpp)

# 1.2 Thêm source của GLAD (File C)
list(APPEND SOURCE_FILES "${GLAD_ROOT}/src/glad.c")

# 1.3 Thêm source của IMGUI (File C++) --- [MỚI] ---
# Gom file core của ImGui (.cpp) và 2 file backend (SDL2 + OpenGL3)
file(GLOB IMGUI_SOURCES 
    "${IMGUI_DIR}/*.cpp"
    "${IMGUI_DIR}/backends/imgui_impl_sdl2.cpp"
    "${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp"
)
list(APPEND SOURCE_FILES ${IMGUI_SOURCES})

# Tạo file chạy
add_executable(main ${SOURCE_FILES})

# =======================
# 2. SETUP INCLUDE (GLOBAL)
# =======================
target_include_directories(main PRIVATE
    ${PROJECT_SOURCE_DIR}/include

    # --- GLAD INCLUDE ---
    ${GLAD_ROOT}/include

    # --- IMGUI INCLUDE --- [MỚI] ---
    ${IMGUI_DIR}           # Để include "imgui.h"
    ${IMGUI_DIR}/backends  # Để include "imgui_impl_sdl2.h"

    # --- SDL INCLUDE (Dùng chung cho cả Win & Mac) ---
    ${SDL_ROOT}/SDL/include
    ${SDL_ROOT}/Image/include
    ${SDL_ROOT}/Mixer/include
    ${SDL_ROOT}/TTF/include
    
    # Fix lỗi header tìm header nội bộ của SDL
    ${SDL_ROOT}/SDL/include/SDL2
    ${SDL_ROOT}/Image/include/SDL2
    ${SDL_ROOT}/Mixer/include/SDL2
    ${SDL_ROOT}/TTF/include/SDL2
)

# =======================
# 3. SETUP LINKING (PLATFORM SPECIFIC)
# =======================

if (APPLE)
    # --- KIỂM TRA 1: Framework hệ thống (/Library/Frameworks) ---
    if (EXISTS "/Library/Frameworks/SDL2.framework")
        message(STATUS "macOS: Found System Frameworks in /Library/Frameworks")
        
        target_link_options(main PRIVATE "-F/Library/Frameworks")
        target_link_libraries(main PRIVATE 
            "-framework SDL2" 
            "-framework SDL2_image" 
            "-framework SDL2_mixer" 
            "-framework SDL2_ttf"
            "-framework OpenGL"  # Quan trọng cho cả ImGui và Glad
            "-framework CoreFoundation" # Bổ sung thêm cho chắc chắn
        )
        
        # Setup RPATH để file chạy tìm thấy Framework khi mang sang máy khác
        set_target_properties(main PROPERTIES BUILD_WITH_INSTALL_RPATH TRUE)
        set_target_properties(main PROPERTIES INSTALL_RPATH "@executable_path/../Frameworks;/Library/Frameworks")

    # --- KIỂM TRA 2: Fallback sang Homebrew ---
    else()
        message(STATUS "macOS: System Frameworks NOT found. Fallback to Homebrew...")
        
        find_package(SDL2 REQUIRED)
        find_package(SDL2_image REQUIRED)
        find_package(SDL2_mixer REQUIRED)
        find_package(SDL2_ttf REQUIRED)
        find_package(OpenGL REQUIRED) 

        target_link_libraries(main PRIVATE 
            SDL2::SDL2 
            SDL2_image::SDL2_image 
            SDL2_mixer::SDL2_mixer 
            SDL2_ttf::SDL2_ttf
            OpenGL::GL
        )
    endif()

elseif (WIN32)
    message(STATUS "Windows: Using Local Vendor Libs")

    # Đường dẫn file .lib
    target_link_directories(main PRIVATE
        ${SDL_ROOT}/SDL/lib
        ${SDL_ROOT}/Image/lib
        ${SDL_ROOT}/Mixer/lib
        ${SDL_ROOT}/TTF/lib
    )

    # Link các thư viện
    target_link_libraries(main PRIVATE 
        SDL2main SDL2 
        SDL2_image SDL2_mixer SDL2_ttf
        opengl32  # BẮT BUỘC cho ImGui OpenGL3 backend
    )

    # Copy DLL ra thư mục build
    add_custom_command(TARGET main POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${SDL_ROOT}/SDL/bin/SDL2.dll
            ${SDL_ROOT}/Image/bin/SDL2_image.dll
            ${SDL_ROOT}/Mixer/bin/SDL2_mixer.dll
            ${SDL_ROOT}/TTF/bin/SDL2_ttf.dll
            $<TARGET_FILE_DIR:main>
    )

else()
    message(FATAL_ERROR "Unsupported platform")
endif()