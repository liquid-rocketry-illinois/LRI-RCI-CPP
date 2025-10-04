cmake_minimum_required(VERSION 3.28)
project(LRI_control_panel)

set(CMAKE_CXX_STANDARD 23)
set(EXE_NAME "LRIRCI")

if (WIN32)
    set(PLATFORM_LIBS "opengl32 gdi32 imm32 user32 advapi32 Setupapi")
else ()
    set(PLATFORM_LIBS "GL")
endif ()

add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/VERSION.cpp
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/VERSION
        COMMAND ${CMAKE_COMMAND} -DSOURCE:STRING=${CMAKE_CURRENT_SOURCE_DIR} -DBIN:STRING=${CMAKE_CURRENT_BINARY_DIR}
        -DBTYPE:STRING=${CMAKE_BUILD_TYPE} -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/gen_version.cmake
)

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
        libs/implot/implot.cpp
        libs/implot/implot_items.cpp
        libs/implot/implot_demo.cpp
)

target_include_directories(imgui PUBLIC libs/imgui libs/imgui/backends libs/implot)
target_link_libraries(imgui PUBLIC glfw)

add_executable(LRI_control_panel
        ${CMAKE_CURRENT_BINARY_DIR}/VERSION.cpp
        src/main.cpp
        src/utils.cpp
        src/improgress.cpp
        src/RCP_Host_Impl.cpp

        # UI Elements
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

        # Interfaces
        src/interfaces/COMPort.cpp
        src/interfaces/IOInterface.cpp
        src/interfaces/TCPSocket.cpp
        src/interfaces/VirtualPort.cpp

        # Hardware classes
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

        # Platform specific
        $<$<BOOL:${WIN32}>:src/WindowsEmbeddedResource.cpp>
        $<$<BOOL:${WIN32}>:resources/resources.rc>
        $<$<BOOL:${WIN32}>:src/interfaces/WindowsCOMPort.cpp>
        $<$<NOT:$<BOOL:${WIN32}>>:src/interfaces/LinuxSerialPort.cpp>
        $<$<NOT:$<BOOL:${WIN32}>>:src/LinuxEmbeddedResource.cpp>
)

target_link_libraries(LRI_control_panel PRIVATE glfw RCP-Host nlohmann_json SFML::Network imgui ${PLATFORM_LIBS})

target_include_directories(LRI_control_panel PRIVATE include)

target_compile_definitions(LRI_control_panel PRIVATE -DNOMINMAX)

# Statically link C runtime if on windows
if (WIN32)
    set_property(TARGET LRI_control_panel glfw RCP-Host PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif ()

set_target_properties(LRI_control_panel PROPERTIES OUTPUT_NAME "LRIRCI")

get_target_property(EXENAME LRI_control_panel OUTPUT_NAME)
file(COPY targets/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/targets)

if (${CMAKE_BUILD_TYPE} STREQUAL "Release")
    add_custom_target(LRI_control_panel_GithubRelease
            COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/GithubRelease.sh ${CMAKE_CURRENT_BINARY_DIR} ${EXENAME} ${CMAKE_CURRENT_SOURCE_DIR})
    add_dependencies(LRI_control_panel_GithubRelease LRI_control_panel)
endif ()