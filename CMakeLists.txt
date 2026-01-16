cmake_minimum_required(VERSION 3.28)
project(LRI_control_panel)

set(CMAKE_CXX_STANDARD 23)
set(EXE_NAME "LRIRCI")

# Enforce Windows only builds since I don't have time for Linux builds
if(NOT WIN32)
    message(FATAL_ERROR "RCI is currently only available on Windows")
endif()

# Enforce MSVC because GCC is hard and RCI depends on some MSVC features
if(NOT CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    message("Compiler ID is: ${CMAKE_CXX_COMPILER_ID}")
    message(FATAL_ERROR "MSVC is required on windows")
endif()

# Generate VERSION.cpp from a custom script
add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/VERSION.cpp
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/VERSION
        COMMAND ${CMAKE_COMMAND} -DSOURCE:STRING=${CMAKE_CURRENT_SOURCE_DIR} -DBIN:STRING=${CMAKE_CURRENT_BINARY_DIR}
        -DBTYPE:STRING=${CMAKE_BUILD_TYPE} -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/gen_version.cmake
)

# Add needed libraries submoduled in libs/
add_subdirectory(libs/glfw)
add_subdirectory(libs/RCP-Host)
add_subdirectory(libs/json)

# Turn off a bunch of unneeded SFML stuff
set(SFML_BUILD_GRAPHICS OFF)
set(SFML_BUILD_WINDOW OFF)
set(SFML_BUILD_AUDIO OFF)
set(SFML_USE_STATIC_STD_LIBS ON)
set(BUILD_SHARED_LIBS OFF)
set(SFML_INSTALL_PKGCONFIG_FILES OFF)
add_subdirectory(libs/sfml)

# Set up a dedicated ImGui library so it doesn't have to be in the main executables file list
# Also include ImPlot since its related
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

# Setup includes and libraries for ImGui
target_include_directories(imgui PUBLIC libs/imgui libs/imgui/backends libs/implot)
target_link_libraries(imgui PUBLIC glfw)

# Add the main executable target with all the sources
add_executable(LRI_control_panel
        # The generated version info file
        ${CMAKE_CURRENT_BINARY_DIR}/VERSION.cpp

        src/main.cpp
        src/utils.cpp
        src/improgress.cpp
        src/rendering.cpp

        # UI Elements
        src/UI/Sidebar.cpp
        src/UI/UIControl.cpp
#        src/UI/AngledActuatorViewer.cpp
#        src/UI/BoolSensorViewer.cpp
#        src/UI/EStopViewer.cpp
#        src/UI/ErrorWindow.cpp
#        src/UI/PromptViewer.cpp
#        src/UI/RawViewer.cpp
#        src/UI/SensorViewer.cpp
#        src/UI/SimpleActuatorViewer.cpp
#        src/UI/StepperViewer.cpp
#        src/UI/TargetChooser.cpp
#        src/UI/TestStateViewer.cpp
#        src/UI/Windowlet.cpp

        # Interfaces
        src/interfaces/IOInterface.cpp
        src/interfaces/TCPSocket.cpp
        src/interfaces/VirtualPort.cpp

        # Hardware classes
        src/hardware/AngledActuator.cpp
        src/hardware/BoolSensor.cpp
        src/hardware/EventLog.cpp
        src/hardware/HardwareControl.cpp
        src/hardware/Prompt.cpp
        src/hardware/Sensors.cpp
        src/hardware/SimpleActuators.cpp
        src/hardware/Steppers.cpp
        src/hardware/TargetLog.cpp
        src/hardware/TestState.cpp

        # (technically) Platform specific
        src/WindowsEmbeddedResource.cpp
        resources/resources.rc
        src/interfaces/WindowsCOMPort.cpp
)

# Add needed libraries
target_link_libraries(LRI_control_panel PRIVATE RCP-Host nlohmann_json SFML::Network imgui
        opengl32 gdi32 imm32 user32 advapi32 Setupapi Dwmapi)

# Add includes folder
target_include_directories(LRI_control_panel PRIVATE include)

# Stop Windows.h from defining the MIN and MAX macros because they're annoying
target_compile_definitions(LRI_control_panel PRIVATE -DNOMINMAX)

# Enable warnings and warnings-as-errors
target_compile_options(LRI_control_panel PRIVATE /W3 /WX)

# Static link CRT
set_property(TARGET LRI_control_panel glfw RCP-Host PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

# Set EXE name
set_target_properties(LRI_control_panel PROPERTIES OUTPUT_NAME "LRIRCI")

# Add target to copy all the target jsons to the build directory for debugging
add_custom_target(copy_target_jsons ALL
        COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
        ${CMAKE_SOURCE_DIR}/targets
        ${CMAKE_BINARY_DIR}/targets
)

add_dependencies(LRI_control_panel copy_target_jsons)

# If on release build, add the github release script as a target
if(${CMAKE_BUILD_TYPE} STREQUAL "Release")
    get_target_property(EXENAME LRI_control_panel OUTPUT_NAME)
    add_custom_target(LRI_control_panel_GithubRelease
            COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/GithubRelease.sh ${CMAKE_CURRENT_BINARY_DIR} ${EXENAME} ${CMAKE_CURRENT_SOURCE_DIR})
    add_dependencies(LRI_control_panel_GithubRelease LRI_control_panel)
else()
    # If in debug build, define a debug macro for some internal use
    target_compile_definitions(LRI_control_panel PRIVATE -DRCIDEBUG)
endif()
