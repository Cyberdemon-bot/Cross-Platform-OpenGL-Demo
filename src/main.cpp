// 1. Thêm dòng này LÊN ĐẦU TIÊN (Trước khi include bất cứ thứ gì của SDL)
#define SDL_MAIN_HANDLED 

#include "Application.h"

// Hàm main chuẩn của C++
int main(int argc, char* argv[]) {
    Application* app = new Application();

    if (app->Init("My Graphics Engine", 1280, 720)) {
        app->Run();
    }

    delete app;
    return 0;
}