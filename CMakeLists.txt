cmake_minimum_required(VERSION 3.28)
project(LRI_control_panel)

set(CMAKE_CXX_STANDARD 23)

add_subdirectory(libs/glfw)
add_subdirectory(libs/RCP-Host)
add_subdirectory(libs/json)

add_executable(LRI_control_panel
        src/main.cpp
        src/utils.cpp
        src/interfaces/COMPort.cpp
        src/WindowsResource.cpp
        src/RCP_Host_Impl.cpp
        resources/resources.rc
        libs/imgui/imgui.cpp
        libs/imgui/imgui_draw.cpp
        libs/imgui/imgui_tables.cpp
        libs/imgui/imgui_widgets.cpp
        libs/imgui/backends/imgui_impl_glfw.cpp
        libs/imgui/backends/imgui_impl_opengl3.cpp
        src/UI/BaseUI.cpp
        src/UI/TargetChooser.cpp
        src/UI/EStop.cpp
        src/UI/Solenoids.cpp
        src/interfaces/VirtualPort.cpp
        src/UI/TestControl.cpp
)

target_link_libraries(LRI_control_panel PUBLIC glfw opengl32 gdi32 imm32 user32 advapi32 Setupapi RCP-Host nlohmann_json)
target_include_directories(LRI_control_panel PUBLIC libs/glfw/include libs/imgui
        libs/imgui/backends libs/json/include include)

set_property(TARGET LRI_control_panel glfw RCP-Host PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

set_target_properties(LRI_control_panel PROPERTIES OUTPUT_NAME "LRIRCI")

file(COPY targets/ DESTINATION ${CMAKE_BINARY_DIR}/targets)