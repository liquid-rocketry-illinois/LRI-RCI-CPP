IF (NOT ${WIN32})
    return()
ENDIF ()

# Create resources folder in build folder
add_custom_target(resources-folder COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/resources)

set(FONTS font_regular font_bold font_italic)

FOREACH (FONT ${FONTS})
    add_custom_target(${FONT}-raw
            COMMAND ${CMAKE_OBJCOPY} -I binary -O elf64-x86-64 -B i386:x86-64
                ${CMAKE_SOURCE_DIR}/resources/${FONT}.ttf
                ${CMAKE_BINARY_DIR}/resources/${FONT}.o
            DEPENDS ${CMAKE_SOURCE_DIR}/resources/${FONT}.ttf
    )

    add_dependencies(${FONT}-raw resources-folder)

    set(SYM_STR "${CMAKE_SOURCE_DIR}/resources/${FONT}.ttf")
    string(REGEX REPLACE "[^a-zA-Z0-9]" "_" SYM_STR ${SYM_STR})
    string(PREPEND SYM_STR "_binary_")

    add_custom_target(${FONT}
            COMMAND ${CMAKE_OBJCOPY} ${CMAKE_BINARY_DIR}/resources/${FONT}.o ${CMAKE_BINARY_DIR}/resources/${FONT}.o
                --redefine-sym ${SYM_STR}_start=${FONT}_start
                --redefine-sym ${SYM_STR}_end=${FONT}_end
                --redefine-sym ${SYM_STR}_size=${FONT}_size
                --rename-section .data=.rodata,alloc,load,readonly
            BYPRODUCTS ${CMAKE_BINARY_DIR}/resources/${FONT}.o
    )

    add_dependencies(${FONT} ${FONT}-raw)
ENDFOREACH ()

## On linux, create targets to create binary versions of fonts to be included and linked in
#set(REGULAR_SYM_STR ${CMAKE_SOURCE_DIR}/resources/font-regular.ttf)
#message(INFO ${REGULAR_SYM_STR})
#string(REGEX REPLACE "[^a-zA-Z0-9]" "_" REGULAR_SYM_STR ${REGULAR_SYM_STR})
#string(PREPEND REGULAR_SYM_STR "_binary_")
#message(INFO ${REGULAR_SYM_STR})
#
#
## Add targets to generate each object file
#add_custom_target(font-regular
#        COMMAND ${CMAKE_OBJCOPY} -I binary -O elf64-x86-64 -B i386:x86-64 ${CMAKE_SOURCE_DIR}/resources/font-regular.ttf ${CMAKE_BINARY_DIR}/resources/font-regular.o
#        DEPENDS ${CMAKE_SOURCE_DIR}/resources/font-regular.ttf
#        BYPRODUCTS ${CMAKE_BINARY_DIR}/resources/font-regular.o
#)
#add_dependencies(font-regular resources-folder)
#
#add_custom_target(font-bold
#        COMMAND ${CMAKE_OBJCOPY} -I binary -O elf64-x86-64 -B i386:x86-64 ${CMAKE_SOURCE_DIR}/resources/font-bold.ttf ${CMAKE_BINARY_DIR}/resources/font-bold.o
#        DEPENDS ${CMAKE_SOURCE_DIR}/resources/font-bold.ttf
#        BYPRODUCTS ${CMAKE_BINARY_DIR}/resources/font-bold.o
#)
#add_dependencies(font-bold resources-folder)
#
#add_custom_target(font-italic
#        COMMAND ${CMAKE_OBJCOPY} -I binary -O elf64-x86-64 -B i386:x86-64 ${CMAKE_SOURCE_DIR}/resources/font-italic.ttf ${CMAKE_BINARY_DIR}/resources/font-italic.o
#        DEPENDS ${CMAKE_SOURCE_DIR}/resources/font-italic.ttf
#        BYPRODUCTS ${CMAKE_BINARY_DIR}/resources/font-italic.o
#)
#add_dependencies(font-italic resources-folder)