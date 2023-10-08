cmake_minimum_required(VERSION 3.1)

set(GLOBAL_NAME "openmeat")
set(GLOBAL_VERSION "0.0.1")

project(${GLOBAL_NAME} LANGUAGES CXX VERSION ${GLOBAL_VERSION}
        DESCRIPTION "Library to manage Deadmaze connection")

set(BUILD_DISSECTOR ON CACHE BOOL "Build Wireshark Dissector")
set(BUILD_TESTS OFF CACHE BOOL "Build CppUnit tests")

include(GNUInstallDirs)

add_subdirectory("lib/")
if (${BUILD_TESTS})
    add_subdirectory("tests/")
endif()
if (${BUILD_DISSECTOR})
    add_subdirectory("tools/dissector")
endif()
add_subdirectory("tools/hook/")
add_subdirectory("tools/key_finder/")
