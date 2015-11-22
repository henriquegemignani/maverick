CMAKE_MINIMUM_REQUIRED(VERSION 3.0)
cmake_policy(VERSION 3.2.2)

# SOURCE LIST
include (src_list.cmake)

if (UNIX OR MINGW)
    # se for o g++ para *nix ou o MinGW para Windows:
    add_definitions(-Wall -ansi -U_FORTIFY_SOURCE -std=gnu++11)
    if(NOT ANDROID)
        add_definitions(-msse2)
    endif()
endif ()
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    add_definitions(
        -Wno-self-assign
        -Wno-deprecated-register
    )
endif ()
if (MSVC) 
    add_definitions(/D_CRT_SECURE_NO_WARNINGS /W4 /wd4100 /wd4127 /wd4201 /wd4211 /wd4250 /wd4706 /fp:fast /MP)
    # SSE2 is automatic in amd64
    IF(NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
        add_definitions(/arch:SSE2)
    ENDIF()
endif ()

include_directories (${CMAKE_CURRENT_SOURCE_DIR})
include_directories (${CMAKE_CURRENT_BINARY_DIR})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

add_ugdk_executable(maverick ${GAME_SRCS})
target_link_libraries(maverick PUBLIC tiled-reader)

# set(maverick_DATA_PATH "${CMAKE_SOURCE_DIR}/data")

# On windows, copy the .dll files so it can run without annoying anyone.
if(WIN32)
    IF(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(LIBS_SOURCE "${CMAKE_SOURCE_DIR}/lib64/")
    ELSE()
        set(LIBS_SOURCE "${CMAKE_SOURCE_DIR}/lib/")
    ENDIF()
    add_custom_command(TARGET maverick POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${LIBS_SOURCE}" "${CMAKE_CURRENT_BINARY_DIR}"
        COMMENT "Copying libs from ${LIBS_SOURCE} to the maverick's solution directory.")
endif(WIN32)

