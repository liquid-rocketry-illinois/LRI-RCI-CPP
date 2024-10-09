cmake_minimum_required(VERSION 3.28)
project(LRI_control_panel)

set(CMAKE_CXX_STANDARD 23)

add_subdirectory(libs/glfw)

add_executable(LRI_control_panel src/main.cpp libs/imgui/imgui.cpp libs/imgui/imgui_draw.cpp
        libs/imgui/imgui_tables.cpp libs/imgui/imgui_widgets.cpp
        libs/imgui/backends/imgui_impl_glfw.cpp libs/imgui/backends/imgui_impl_opengl3.cpp
        src/setup.cpp)

target_link_libraries(LRI_control_panel PUBLIC glfw opengl32 gdi32 imm32 user32 advapi32)
target_include_directories(LRI_control_panel PUBLIC libs/glfw/include libs/imgui
        libs/imgui/backends include)

set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "LRIRCP")