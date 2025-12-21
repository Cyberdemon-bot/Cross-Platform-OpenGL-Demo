# Cross-Platform OpenGL Demo 

A minimal OpenGL demo project with SDL2 + Imgui + Glad, configured for both macOS and Windows using CMake.

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

### macOS Setup

You have **two options** for installing SDL2 on macOS:

#### **Option 1: Using Homebrew (Recommended)**

If you have Homebrew installed:

```bash
# Install SDL2
brew install sdl2

# Verify installation
brew list sdl2
```

#### **Option 2: Manual Installation (No Homebrew)**

If you don't have Homebrew or prefer manual installation:
   - Open dmg files at vender/SDL2/framework_dmg
   - Open Finder
   - Press `Cmd + Shift + G`
   - Type: `~/Library/Frameworks`
   - If the folder doesn't exist, create it
   - Drag `*.framework` from the mounted DMG into this folder

### Windows Setup

**All dependencies are already included!** No additional installation needed.

## 🔧 Building the Project

### Step 1: Generate Build Files

Open terminal (macOS) or Command Prompt (Windows) in the project root:

```bash
cmake -S . -B build
```

```bash
cmake -S . -B build -A x64
```
This ensures 64-bit build configuration.

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
