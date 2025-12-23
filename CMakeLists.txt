cmake_minimum_required(VERSION 3.10)
project(MySDLGame LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# =======================
# 0. DEFINITIONS
# =======================
set(VENDOR_DIR  ${PROJECT_SOURCE_DIR}/vendor)
set(SDL_ROOT    ${VENDOR_DIR}/SDL2)
set(GLAD_ROOT   ${VENDOR_DIR}/glad)
set(IMGUI_DIR   ${VENDOR_DIR}/imgui)
set(ASSIMP_ROOT ${VENDOR_DIR}/assimp) # <--- [MỚI] Định nghĩa Assimp

# =======================
# 1. SETUP SOURCES
# =======================
file(GLOB SOURCE_FILES CONFIGURE_DEPENDS src/*.cpp)
list(APPEND SOURCE_FILES "${GLAD_ROOT}/src/glad.c")

file(GLOB IMGUI_SOURCES 
    "${IMGUI_DIR}/*.cpp"
    "${IMGUI_DIR}/backends/imgui_impl_sdl2.cpp"
    "${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp"
)
list(APPEND SOURCE_FILES ${IMGUI_SOURCES})

add_executable(main ${SOURCE_FILES})

# =======================
# 2. SETUP INCLUDE (GLOBAL)
# =======================
target_include_directories(main PRIVATE
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/vendor

    # --- GLAD ---
    ${GLAD_ROOT}/include

    # --- IMGUI ---
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends

    # --- ASSIMP [MỚI] ---
    ${ASSIMP_ROOT}/include

    # --- SDL ---
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
# 3. SETUP LINKING (PLATFORM SPECIFIC)
# =======================

if (APPLE)
    # --- ASSIMP CONFIG CHO MAC ---
    # Tìm file .dylib trong folder lib (Ví dụ: libassimp.dylib hoặc libassimp.6.0.2.dylib)
    file(GLOB ASSIMP_MAC_LIB "${ASSIMP_ROOT}/lib/*.dylib")
    
    # Setup RPATH: Giúp file chạy tìm thấy thư viện nằm ở vendor/assimp/lib
    set_target_properties(main PROPERTIES BUILD_RPATH "${ASSIMP_ROOT}/lib")
    set_target_properties(main PROPERTIES INSTALL_RPATH "@executable_path/../vendor/assimp/lib")
    
    # --- SDL CONFIG CHO MAC ---
    if (EXISTS "/Library/Frameworks/SDL2.framework")
        message(STATUS "macOS: Linking System Frameworks")
        target_link_options(main PRIVATE "-F/Library/Frameworks")
        target_link_libraries(main PRIVATE 
            "-framework SDL2" 
            "-framework SDL2_image" 
            "-framework SDL2_mixer" 
            "-framework SDL2_ttf"
            "-framework OpenGL"
            "-framework CoreFoundation"
            ${ASSIMP_MAC_LIB} # <--- Link Assimp vào đây
        )
    else()
        message(STATUS "macOS: Linking Homebrew/Local Libs")
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
            ${ASSIMP_MAC_LIB} # <--- Link Assimp vào đây
        )
    endif()

elseif (WIN32)
    message(STATUS "Windows: Linking Local Vendor Libs")

    # --- ASSIMP CONFIG CHO WIN ---
    # 1. Tự động tìm file .lib trong folder lib (Để tránh việc phải gõ đúng tên assimp-vc143-mt.lib)
    file(GLOB ASSIMP_WIN_LIB "${ASSIMP_ROOT}/lib/*.lib")
    
    # 2. Tự động tìm file .dll trong folder bin
    file(GLOB ASSIMP_WIN_DLL "${ASSIMP_ROOT}/bin/*.dll")

    target_link_directories(main PRIVATE
        ${SDL_ROOT}/SDL/lib
        ${SDL_ROOT}/Image/lib
        ${SDL_ROOT}/Mixer/lib
        ${SDL_ROOT}/TTF/lib
        ${ASSIMP_ROOT}/lib # Thêm đường dẫn lib của Assimp
    )

    target_link_libraries(main PRIVATE 
        SDL2main SDL2 
        SDL2_image SDL2_mixer SDL2_ttf
        opengl32
        ${ASSIMP_WIN_LIB} # <--- Link file .lib của Assimp
    )

    # Copy DLL (SDL + Assimp) ra thư mục build
    add_custom_command(TARGET main POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${SDL_ROOT}/SDL/bin/SDL2.dll
            ${SDL_ROOT}/Image/bin/SDL2_image.dll
            ${SDL_ROOT}/Mixer/bin/SDL2_mixer.dll
            ${SDL_ROOT}/TTF/bin/SDL2_ttf.dll
            ${ASSIMP_WIN_DLL} # <--- Copy file .dll của Assimp
            $<TARGET_FILE_DIR:main>
    )

else()
    message(FATAL_ERROR "Unsupported platform")
endif()
