cmake_minimum_required(VERSION 3.22)
project(XExample)

set(CMAKE_CXX_STANDARD 17)
set(RENDER_TARGET_NAME XRender)
set(TARGET_NAME XExample)
set(THIRD_PARTY_DIR ${PROJECT_SOURCE_DIR}/3rdparty)
set(SHADER_COMPILE_TARGET ShaderCompile)
set(vulkan_include ${THIRD_PARTY_DIR}/VulkanSDK/include)
set(assimp_include ${THIRD_PARTY_DIR}/assimp/include)

if(APPLE)
    set(vulkan_lib ${THIRD_PARTY_DIR}/VulkanSDK/lib/MacOS/libvulkan.1.dylib)
    set(assimp_lib ${THIRD_PARTY_DIR}/assimp/lib/MacOS/arm64/libassimp.5.dylib)
    set(glslangValidator_executable ${THIRD_PARTY_DIR}/VulkanSDK/bin/MacOS/glslangValidator)
    set(GLFW_PLATFORM "COCOA")
    add_compile_definitions("VK_LAYER_PATH=${THIRD_PARTY_DIR}/VulkanSDK/bin/MacOS")
    add_compile_definitions("VK_ICD_FILENAMES=${THIRD_PARTY_DIR}/VulkanSDK/bin/MacOS/MoltenVK_icd.json")
elseif(WIN32)
    set(vulkan_lib ${THIRD_PARTY_DIR}/VulkanSDK/lib/Win32/vulkan-1.lib)
    set(glslangValidator_executable ${THIRD_PARTY_DIR}/VulkanSDK/bin/Win32/glslangValidator.exe)
    set(GLFW_PLATFORM "WIN32")
    add_compile_definitions("VK_LAYER_PATH=${THIRD_PARTY_DIR}/VulkanSDK/bin/Win32")
elseif(UNIX AND NOT APPLE)
#    set(vulkan_lib ${THIRD_PARTY_DIR}/VulkanSDK/lib/Linux/libvulkan.so.1)
#    set(glslangValidator_executable ${THIRD_PARTY_DIR}/VulkanSDK/bin/Linux/glslangValidator)
    set(GLFW_PLATFORM "X11")
    add_compile_definitions("VK_LAYER_PATH=${THIRD_PARTY_DIR}/VulkanSDK/bin/Linux")
else()
    message(FATAL_ERROR "Unknown Platform")
endif()

#configure_file(glfw_config.h.in glfw_config.h @ONLY)

add_subdirectory(3rdparty)
add_subdirectory(shaders)
add_subdirectory(source)

#add_library(${TARGET_NAME} ${HEADER_FILES} ${SOURCE_FILES})
# 查找 include 目录下的所有 .h 文件
file(GLOB_RECURSE RENDER_INCLUDES "include/*.h")

# 去掉路径中的根目录
foreach(INCLUDE_PATH ${RENDER_INCLUDES})
    # 将根目录替换为空字符串
    string(REPLACE "${CMAKE_SOURCE_DIR}/include" "" INCLUDE_PATH ${INCLUDE_PATH})
    # 将修改后的路径添加到列表中
    list(APPEND RENDER_INCLUDES_NEW ${INCLUDE_PATH})
endforeach()

# 将新的列表赋值给 RENDER_INCLUDES
#set(RENDER_INCLUDES ${RENDER_INCLUDES_NEW})
#message(${RENDER_INCLUDES})

include_directories(include)
add_executable(XExample main.cpp)
add_dependencies(${RENDER_TARGET_NAME} ${SHADER_COMPILE_TARGET})

#target_include_directories(${TARGET_NAME} PUBLIC $<BUILD_INTERFACE:${RENDER_INCLUDES}>)

target_link_libraries(${TARGET_NAME} PUBLIC ${RENDER_TARGET_NAME})


