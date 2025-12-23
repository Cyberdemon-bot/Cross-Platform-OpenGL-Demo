cmake_minimum_required(VERSION 3.10)
project(MySDLGame LANGUAGES CXX C)

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
set(SDL_ROOT    ${VENDOR_DIR}/SDL2)
set(GLAD_ROOT   ${VENDOR_DIR}/glad)
set(IMGUI_DIR   ${VENDOR_DIR}/imgui)
set(ASSIMP_ROOT ${VENDOR_DIR}/assimp)

# =======================
# 2. SETUP SOURCES
# =======================
# 2.1 Source Game
file(GLOB SOURCE_FILES CONFIGURE_DEPENDS src/*.cpp)

# 2.2 Source Glad
list(APPEND SOURCE_FILES "${GLAD_ROOT}/src/glad.c")

# 2.3 Source ImGui
file(GLOB IMGUI_SOURCES 
    "${IMGUI_DIR}/*.cpp"
    "${IMGUI_DIR}/backends/imgui_impl_sdl2.cpp"
    "${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp"
)
list(APPEND SOURCE_FILES ${IMGUI_SOURCES})

# Tạo file thực thi
add_executable(main ${SOURCE_FILES})

# =======================
# 3. SETUP INCLUDE (GLOBAL)
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

    # SDL2
    ${SDL_ROOT}/SDL/include
    ${SDL_ROOT}/Image/include
    ${SDL_ROOT}/Mixer/include
    ${SDL_ROOT}/TTF/include
    ${SDL_ROOT}/SDL/include/SDL2
    ${SDL_ROOT}/Image/include/SDL2
    ${SDL_ROOT}/Mixer/include/SDL2
    ${SDL_ROOT}/TTF/include/SDL2
)

# =======================
# 4. SETUP LINKING (PLATFORM SPECIFIC)
# =======================

if (APPLE)
    # ---------------------------------------------------------
    # macOS CONFIGURATION
    # ---------------------------------------------------------
    
    # Tìm file thư viện .dylib của Assimp
    file(GLOB ASSIMP_MAC_LIB "${ASSIMP_ROOT}/lib/*.dylib")
    
    # [FIX QUAN TRỌNG]: Sửa lỗi "Library not loaded"
    # Chúng ta set RPATH chứa CẢ đường dẫn Assimp VÀ đường dẫn Framework hệ thống
    # Dấu chấm phẩy (;) dùng để ngăn cách các đường dẫn trong danh sách
    set_target_properties(main PROPERTIES BUILD_RPATH "${ASSIMP_ROOT}/lib;/Library/Frameworks")
    set_target_properties(main PROPERTIES INSTALL_RPATH "@executable_path/../vendor/assimp/lib;/Library/Frameworks")

    # Kiểm tra SDL2 System Frameworks
    if (EXISTS "/Library/Frameworks/SDL2.framework")
        message(STATUS "macOS: Found System Frameworks in /Library/Frameworks")
        
        target_link_options(main PRIVATE "-F/Library/Frameworks")
        target_link_libraries(main PRIVATE 
            "-framework SDL2" 
            "-framework SDL2_image" 
            "-framework SDL2_mixer" 
            "-framework SDL2_ttf"
            "-framework OpenGL"
            "-framework CoreFoundation"
            ${ASSIMP_MAC_LIB} # Link Assimp
        )
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
            ${ASSIMP_MAC_LIB} # Link Assimp
        )
    endif()

elseif (WIN32)
    # ---------------------------------------------------------
    # Windows CONFIGURATION
    # ---------------------------------------------------------
    message(STATUS "Windows: Using Local Vendor Libs")

    # 1. Tìm file .lib và .dll của Assimp
    file(GLOB ASSIMP_WIN_LIB "${ASSIMP_ROOT}/lib/*.lib")
    file(GLOB ASSIMP_WIN_DLL "${ASSIMP_ROOT}/bin/*.dll")

    # 2. Thêm đường dẫn tìm thư viện
    target_link_directories(main PRIVATE
        ${SDL_ROOT}/SDL/lib
        ${SDL_ROOT}/Image/lib
        ${SDL_ROOT}/Mixer/lib
        ${SDL_ROOT}/TTF/lib
        ${ASSIMP_ROOT}/lib
    )

    # 3. Link thư viện
    target_link_libraries(main PRIVATE 
        SDL2main SDL2 
        SDL2_image SDL2_mixer SDL2_ttf
        opengl32
        ${ASSIMP_WIN_LIB} # Link file .lib của Assimp
    )

    # 4. Tự động Copy tất cả DLL cần thiết ra cạnh file .exe
    add_custom_command(TARGET main POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${SDL_ROOT}/SDL/bin/SDL2.dll
            ${SDL_ROOT}/Image/bin/SDL2_image.dll
            ${SDL_ROOT}/Mixer/bin/SDL2_mixer.dll
            ${SDL_ROOT}/TTF/bin/SDL2_ttf.dll
            ${ASSIMP_WIN_DLL} # Copy file .dll của Assimp
            $<TARGET_FILE_DIR:main>
    )

else()
    message(FATAL_ERROR "Unsupported platform")
endif()
