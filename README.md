# Cross-Platform OpenGL Demo 

A minimal OpenGL demo project with GLFW + Imgui + GLAD + GLM + Assimp, configured for both macOS and Windows using CMake.

## ✨ Features

- **Cross-Platform**: Works on macOS and Windows
- **Modern OpenGL**: OpenGL 3.3+ core profile
- **CMake Build System**: Simple and portable build configuration

## 📋 Requirements

### macOS
- macOS 10.12 or later
- Xcode Command Line Tools
- CMake 3.10 or higher

### Windows
- Windows 7 or later
- Visual Studio build tools
- CMake 3.10 or higher

## 🚀 Installation

**All dependencies are already included!**  No additional installation needed.
**note: on MacOS, open terminal, go to your project's folder, run this command to disable MacOS security mechanism on assimp .dylib file.
```bash
sudo xattr -cr vendor/assimp/lib/libassimp.6.dylib
```

## 🔧 Building the Project

### Step 1: Generate Build Files

Open terminal (macOS) or Command Prompt (Windows) in the project root:

```bash
cmake -S . -B build
```

```bash
cmake -S . -B build -A x64
```

### Step 2: Build the Project

```bash
cmake --build build
```

**For Release build (optimized):**
```bash
cmake --build build --config Release
```

## ▶️ Running the Demo

### macOS

```bash
./build/main
```

Or if you built with a specific configuration:
```bash
./build/Debug/main
./build/Release/main
```

### Windows

```bash
build\Debug\main.exe
```

Or for Release build:
```bash
build\Release\main.exe
```

**Note:** The SDL2.dll is automatically copied to the output directory by CMake, so the executable will run without additional setup.
