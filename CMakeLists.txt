cmake_minimum_required(VERSION 3.28)
project(LRI_control_panel)

set(CMAKE_CXX_STANDARD 23)
set(EXE_NAME "LRIRCI")

add_subdirectory(libs/glfw)
add_subdirectory(libs/RCP-Host)
add_subdirectory(libs/json)

set(SFML_BUILD_GRAPHICS OFF)
set(SFML_BUILD_WINDOW OFF)
set(SFML_BUILD_AUDIO OFF)
set(SFML_USE_STATIC_STD_LIBS ON)
set(BUILD_SHARED_LIBS OFF)
set(SFML_INSTALL_PKGCONFIG_FILES OFF)
add_subdirectory(libs/sfml)

add_executable(LRI_control_panel
        resources/resources.rc
        libs/imgui/imgui.cpp
        libs/imgui/imgui_draw.cpp
        libs/imgui/imgui_tables.cpp
        libs/imgui/imgui_widgets.cpp
        libs/imgui/imgui_demo.cpp
        libs/imgui/backends/imgui_impl_glfw.cpp
        libs/imgui/backends/imgui_impl_opengl3.cpp
        libs/implot/implot.cpp
        libs/implot/implot_items.cpp
        libs/implot/implot_demo.cpp
        src/main.cpp
        src/utils.cpp
        src/improgress.cpp
        src/interfaces/COMPort.cpp
        src/WindowsResource.cpp
        src/RCP_Host_Impl.cpp
        src/UI/BaseUI.cpp
        src/UI/TargetChooser.cpp
        src/UI/EStopViewer.cpp
        src/UI/SolenoidViewer.cpp
        src/interfaces/VirtualPort.cpp
        src/UI/TestStateViewer.cpp
        src/UI/StepperViewer.cpp
        src/UI/SensorViewer.cpp
        src/UI/RawViewer.cpp
        src/interfaces/TCPSocket.cpp
        src/UI/PromptViewer.cpp
        src/hardware/Solenoids.cpp
        src/hardware/HardwareQualifier.cpp
        src/hardware/Steppers.cpp
        src/hardware/Sensors.cpp
        src/hardware/RawData.cpp
        src/hardware/EStop.cpp
        src/hardware/TestState.cpp
        src/hardware/Prompt.cpp
)

target_link_libraries(LRI_control_panel PUBLIC glfw opengl32 gdi32 imm32 user32 advapi32 Setupapi RCP-Host
        nlohmann_json SFML::Network)
target_include_directories(LRI_control_panel PUBLIC libs/glfw/include libs/imgui libs/imgui/backends libs/json/include
        libs/implot include)

set_property(TARGET LRI_control_panel glfw RCP-Host PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
set_target_properties(LRI_control_panel PROPERTIES OUTPUT_NAME "LRIRCI")

get_target_property(EXENAME LRI_control_panel OUTPUT_NAME)
file(COPY targets/ DESTINATION ${CMAKE_BINARY_DIR}/targets)

if(${CMAKE_BUILD_TYPE} STREQUAL "Release")
    add_custom_target(LRI_control_panel_GithubRelease
            COMMAND ${CMAKE_SOURCE_DIR}/GithubRelease.sh ${CMAKE_BINARY_DIR} ${EXENAME})
    add_dependencies(LRI_control_panel_GithubRelease LRI_control_panel)
endif()