file(GLOB_RECURSE SOURCE_FILES "./*.cpp")
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${SOURCE_FILES})
include_directories(../include)
add_library(${RENDER_TARGET_NAME} STATIC ${SOURCE_FILES})
target_link_libraries(${RENDER_TARGET_NAME} PUBLIC ${vulkan_lib})
target_link_libraries(${RENDER_TARGET_NAME} PUBLIC glm)
target_link_libraries(${RENDER_TARGET_NAME} PUBLIC glfw)

target_include_directories(${RENDER_TARGET_NAME} PUBLIC $<BUILD_INTERFACE:${vulkan_include}>)