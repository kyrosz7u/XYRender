#include <iostream>
#include "macros.h"
#include "window/Window.h"

int main() {
    WindowCreateInfo windowCreateInfo;

    Window window;

    window.initialize(windowCreateInfo);

    while(!window.shouldClose())
    {
        window.pollEvents();
    }
    

    LOG_INFO("12345")
    LOG_DEBUG("67890")
    LOG_ERROR("abcde%s","aaa")
    std::cout << "Hello, World!" << std::endl;
    return 0;
}
