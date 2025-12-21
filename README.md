# opengl-demo
Just a cross-platform setup for opengl + sdl2 + imgui!
⚙️ Setup & Dependencies
🪟 Windows
No setup required. All dependencies are pre-bundled in the vendor/ directory.

🍎 macOS
Choose one of the following methods to install SDL2:

Option A (Recommended): Install via Homebrew:

Bash

brew install sdl2
Option B (No Admin/Manual):

Go to vendor/SDL2/framework_dmg/.

Mount the .dmg file.

Copy SDL2.framework to ~/Library/Frameworks/.

🛠 Build Instructions
Run these commands from the project root:

1. Configure (Creates the build folder automatically):

Bash

cmake -S . -B build
2. Build:

Bash

cmake --build build
▶️ Run
macOS / Linux: ./build/OpenGLDemo

Windows: .\build\Debug\OpenGLDemo.exe
