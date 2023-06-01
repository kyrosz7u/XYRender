add_library(imgui STATIC imgui.cpp imgui_draw.cpp imgui_widgets.cpp imgui_tables.cpp)
target_include_directories(imgui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})