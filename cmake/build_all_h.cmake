
macro(APPEND_INCLUDES target_var source_var prefix)
    set(${target_var} ${${target_var}} "\n" ${prefix})

    foreach(value ${${source_var}})
        set(${target_var} ${${target_var}} "#include <" ${value} ">\n")
    endforeach()
endmacro()

macro(BUILD_ALL_H)

    set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

    file(GLOB CORE_HEADERS RELATIVE ${INCLUDE_DIR} ${INCLUDE_DIR}/vsg/core/*.h )
    file(GLOB MATHS_HEADERS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}/include include/vsg/maths/*.h )
    file(GLOB NODES_HEADERS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}/include include/vsg/nodes/*.h )
    file(GLOB THREADING_HEADERS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}/include include/vsg/threading/*.h )
    file(GLOB UI_HEADERS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}/include include/vsg/ui/*.h )
    file(GLOB UTILS_HEADERS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}/include include/vsg/utils/*.h )
    file(GLOB VIEWER_HEADERS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}/include include/vsg/viewer/*.h )
    file(GLOB IO_HEADERS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}/include include/vsg/io/*.h )
    file(GLOB VK_HEADERS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}/include include/vsg/vk/*.h )
    file(GLOB COMMANDS_HEADERS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}/include include/vsg/commands/*.h )
    file(GLOB STATE_HEADERS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}/include include/vsg/state/*.h )
    file(GLOB TEXT_HEADERS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}/include include/vsg/text/*.h )
    file(GLOB RAYTRACING_HEADERS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}/include include/vsg/raytracing/*.h )
    file(GLOB RTX_HEADERS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}/include include/vsg/rtx/*.h )

    file(READ ${CMAKE_CURRENT_SOURCE_DIR}/cmake/header_license_preamble.txt ALL_H_CONTENTS)
    APPEND_INCLUDES(ALL_H_CONTENTS CORE_HEADERS "// Core header files\n")
    APPEND_INCLUDES(ALL_H_CONTENTS MATHS_HEADERS "// Maths header files\n")
    APPEND_INCLUDES(ALL_H_CONTENTS NODES_HEADERS "// Node header files\n")
    APPEND_INCLUDES(ALL_H_CONTENTS COMMANDS_HEADERS "// Commands header files\n")
    APPEND_INCLUDES(ALL_H_CONTENTS STATE_HEADERS "// State header files\n")
    APPEND_INCLUDES(ALL_H_CONTENTS THREADING_HEADERS "// Threading header files\n")
    APPEND_INCLUDES(ALL_H_CONTENTS UI_HEADERS "// User Interface abstraction header files\n")
    APPEND_INCLUDES(ALL_H_CONTENTS VIEWER_HEADERS "// Viewer header files\n")
    APPEND_INCLUDES(ALL_H_CONTENTS VK_HEADERS "// Vulkan related header files\n")
    APPEND_INCLUDES(ALL_H_CONTENTS IO_HEADERS "// Input/Output header files\n")
    APPEND_INCLUDES(ALL_H_CONTENTS UTILS_HEADERS "// Utility header files\n")
    APPEND_INCLUDES(ALL_H_CONTENTS TEXT_HEADERS "// Text header files\n")
    APPEND_INCLUDES(ALL_H_CONTENTS RAYTRACING_HEADERS "// Ray tracing header files\n")
    APPEND_INCLUDES(ALL_H_CONTENTS RTX_HEADERS "// RTX mesh  header files\n")

    file(WRITE include/vsg/all.h ${ALL_H_CONTENTS})

endmacro()

BUILD_ALL_H()
