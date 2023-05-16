#include <iostream>
#include "macros.h"
//#include "vulkan/vulkan_context.h"
//#include "vulkan/vulkan_manager.h"
//#include "vulkan/vulkan_renderpass.h"
int main() {
    LOG_INFO("12345")
    LOG_DEBUG("67890")
    LOG_ERROR("abcde%s","aaa")
    std::cout << "Hello, World!" << std::endl;
    return 0;
}
