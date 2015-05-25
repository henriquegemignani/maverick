CMAKE_MINIMUM_REQUIRED(VERSION 3.0)
cmake_policy(VERSION 3.2.2)
PROJECT(maverick CXX C)

IF(MSVC AND (CMAKE_SIZEOF_VOID_P EQUAL 8))
	set_property(GLOBAL PROPERTY FIND_LIBRARY_USE_LIB64_PATHS ON)
ENDIF()

if((NOT ${CMAKE_CURRENT_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_BINARY_DIR}) AND (NOT EXISTS "${CMAKE_CURRENT_BINARY_DIR}/.gitignore"))
    # Don't polute the git repository with the generated files for out-of-source builds
    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/.gitignore "*")
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules/")

MESSAGE("== Maverick ==")

find_package(Git)

# FindUGDK
option(MAVERICK_INTERNAL_UGDK   "Set to ON to use an internal UGDK instead of the system one." ON)
option(MAVERICK_FORCE_SUBMODULE "Set to ON to force git submodule update whenever CMake is ran." ON)
if(NOT MAVERICK_INTERNAL_UGDK)
    # find_package(UGDK)
    # if(NOT UGDK_FOUND)
        # set(MAVERICK_INTERNAL_UGDK ON CACHE BOOL "Set to ON to use an internal UGDK instead of the system one." FORCE)
        # message(FATAL_ERROR "Unable to find UGDK, forcing internal. Please, re-run CMake.")
    # endif(NOT UGDK_FOUND)
    # include_directories(${UGDK_INCLUDE_DIRS})
    # set(UGDK_EXTERNAL_LIBRARIES ${UGDK_LIBRARIES})
    
else(NOT MAVERICK_INTERNAL_UGDK)
    if(MAVERICK_FORCE_SUBMODULE OR NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/externals/ugdk/CMakeLists.txt")
        if(GIT_FOUND)
            execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init
                            WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
        else(GIT_FOUND)
            message(FATAL_ERROR "Git submodule 'externals/ugdk' not initialized and git not found.")
        endif(GIT_FOUND)
    endif()
    
    set(UGDK_BUILD_TESTS    OFF CACHE BOOL "Set to ON to enable building tests.")
    set(UGDK_BUILD_EXAMPLES OFF CACHE BOOL "Set to ON to enable building examples.")
    set(UGDK_CREATE_BINDINGS OFF CACHE BOOL "Set to ON to generate bindings for supported script languages.")

    add_definitions(-DEMBBEDED_UGDK="${CMAKE_CURRENT_BINARY_DIR}/externals/ugdk")
    add_subdirectory(externals/ugdk EXCLUDE_FROM_ALL)
    include_directories(${UGDK_EXTERNAL_INCLUDE_DIRS})
    set(UGDK_EXTERNAL_LIBRARIES libugdk)
endif(NOT MAVERICK_INTERNAL_UGDK)

add_subdirectory(src) # The game binary.

#add_ugdk_android_project(maverick "Maverick" "org.gemignani.maverick")

