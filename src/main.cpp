#include "Application.h"

// Không cần #define SDL_MAIN_HANDLED nữa

int main(int argc, char* argv[]) {
    Application* app = new Application();

    if (app->Init("My Graphics Engine (GLFW)", 1280, 720)) {
        app->Run();
    }

    delete app;
    return 0;
}
