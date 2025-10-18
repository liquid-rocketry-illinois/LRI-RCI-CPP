IF (${WIN32})
    return()
ENDIF ()

# List of things in the resources folder to bundle
set(RESOURCES font_regular.ttf font_bold.ttf font_italic.ttf LRI_Logo.png LRI_Logo_big.png)

# Create resources folder in build folder
add_custom_target(resources-folder COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/resources)

# Iterate throguh all of the sources above
FOREACH (RESOURCE ${RESOURCES})
    # Strip the extension
    get_filename_component(NOEXT "resources/${RESOURCE}" NAME_WLE)

    # Create the raw binaried resource
    add_custom_command(
            OUTPUT ${CMAKE_BINARY_DIR}/resources/${NOEXT}.raw
            DEPENDS ${CMAKE_SOURCE_DIR}/resources/${RESOURCE} resources-folder
            COMMAND ${CMAKE_OBJCOPY} -I binary -O elf64-x86-64 -B i386:x86-64
            ${CMAKE_SOURCE_DIR}/resources/${RESOURCE}
            ${CMAKE_BINARY_DIR}/resources/${NOEXT}.raw
    )

    # The objcopy command copies the file path of the file we are using into the symbol name, so this and the next
    # command are used to change those symbol names to the name of the file, barring the extension
    set(SYM_STR "${CMAKE_SOURCE_DIR}/resources/${RESOURCE}")
    string(REGEX REPLACE "[^a-zA-Z0-9]" "_" SYM_STR ${SYM_STR})
    string(PREPEND SYM_STR "_binary_")

    add_custom_command(
            OUTPUT ${CMAKE_BINARY_DIR}/resources/${NOEXT}.o
            DEPENDS ${CMAKE_BINARY_DIR}/resources/${NOEXT}.raw
            COMMAND ${CMAKE_OBJCOPY} ${CMAKE_BINARY_DIR}/resources/${NOEXT}.raw ${CMAKE_BINARY_DIR}/resources/${NOEXT}.o
            --redefine-sym ${SYM_STR}_start=${NOEXT}_start
            --redefine-sym ${SYM_STR}_end=${NOEXT}_end
            --redefine-sym ${SYM_STR}_size=${NOEXT}_size
    )

    # Add the created source file to the object library
    target_sources(LRI_control_panel PUBLIC ${CMAKE_BINARY_DIR}/resources/${NOEXT}.o)
ENDFOREACH ()
