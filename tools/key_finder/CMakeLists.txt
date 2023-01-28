FILE(GLOB key_finder_src *.cpp)

project(${GLOBAL_NAME}_key_finder)

add_executable(${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME} ${GLOBAL_NAME})

add_compile_options(
    -Wall
    -03
    $<$<CONFIG:DEBUG>:-g>
)

target_sources(${PROJECT_NAME} PRIVATE ${key_finder_src})
