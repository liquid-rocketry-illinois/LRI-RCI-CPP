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

add_library(imgui OBJECT
        libs/imgui/imgui.cpp
        libs/imgui/imgui_draw.cpp
        libs/imgui/imgui_tables.cpp
        libs/imgui/imgui_widgets.cpp
        libs/imgui/imgui_demo.cpp
        libs/imgui/backends/imgui_impl_glfw.cpp
        libs/imgui/backends/imgui_impl_opengl3.cpp
)

target_include_directories(imgui PUBLIC libs/imgui libs/imgui/backends libs/glfw/include)
target_link_libraries(imgui PUBLIC glfw)

add_library(implot OBJECT
        libs/implot/implot.cpp
        libs/implot/implot_items.cpp
        libs/implot/implot_demo.cpp
)

target_include_directories(implot PUBLIC libs/implot)
target_link_libraries(implot PUBLIC imgui)

add_executable(LRI_control_panel
        src/main.cpp
        src/utils.cpp
        src/improgress.cpp
        src/RCP_Host_Impl.cpp
        $<$<BOOL:${WIN32}>:src/WindowsResource.cpp>
        $<$<BOOL:${WIN32}>:resources/resources.rc>
        $<$<BOOL:${WIN32}>:src/interfaces/WindowsCOMPort.cpp>
        $<$<NOT:$<BOOL:${WIN32}>>:src/interfaces/LinuxCOMPort.cpp>
        src/interfaces/ICOMPort.cpp
        src/interfaces/TCPSocket.cpp
        src/interfaces/VirtualPort.cpp
        src/hardware/AngledActuator.cpp
        src/hardware/BoolSensor.cpp
        src/hardware/EStop.cpp
        src/hardware/HardwareQualifier.cpp
        src/hardware/Prompt.cpp
        src/hardware/RawData.cpp
        src/hardware/Sensors.cpp
        src/hardware/SimpleActuators.cpp
        src/hardware/Steppers.cpp
        src/hardware/TestState.cpp
        src/UI/AngledActuatorViewer.cpp
        src/UI/BoolSensorViewer.cpp
        src/UI/EStopViewer.cpp
        src/UI/PromptViewer.cpp
        src/UI/RawViewer.cpp
        src/UI/SensorViewer.cpp
        src/UI/SimpleActuatorViewer.cpp
        src/UI/StepperViewer.cpp
        src/UI/TargetChooser.cpp
        src/UI/TestStateViewer.cpp
        src/UI/Windowlet.cpp
)

target_link_libraries(LRI_control_panel PRIVATE
        $<$<BOOL:${WIN32}>:opengl32 gdi32 imm32 user32 advapi32 Setupapi>
        $<$<NOT:$<BOOL:${WIN32}>>:GL>
        RCP-Host nlohmann_json imgui implot SFML::Network
)

target_include_directories(LRI_control_panel PRIVATE include)

add_custom_target(copy_target_jsons ALL
        COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
        ${CMAKE_SOURCE_DIR}/targets
        ${CMAKE_BINARY_DIR}/targets
)

add_dependencies(LRI_control_panel copy_target_jsons)

IF(WIN32)
    set_property(TARGET LRI_control_panel glfw RCP-Host PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    add_compile_definitions(-DNOMINMAX)
ENDIF()

set_target_properties(LRI_control_panel PROPERTIES OUTPUT_NAME "LRIRCI")


if(${CMAKE_BUILD_TYPE} STREQUAL "Release")
    get_target_property(EXENAME LRI_control_panel OUTPUT_NAME)
    add_custom_target(LRI_control_panel_GithubRelease
            COMMAND ${CMAKE_SOURCE_DIR}/GithubRelease.sh ${CMAKE_BINARY_DIR} ${EXENAME})
    add_dependencies(LRI_control_panel_GithubRelease LRI_control_panel)
endif()