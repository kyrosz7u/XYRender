file(GLOB_RECURSE SOURCE_FILES "./src/*.cpp")
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${SOURCE_FILES})
include_directories(./include)
add_library(imgui STATIC ${SOURCE_FILES})