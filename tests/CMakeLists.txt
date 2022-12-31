FILE(GLOB tests_source *.cpp)

project(${GLOBAL_NAME}_tests)

add_executable(${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME} ${GLOBAL_NAME} cppunit)

add_compile_options(
    -Wall
    -03
    $<$<CONFIG:DEBUG>:-g>
)

target_sources(${PROJECT_NAME} PRIVATE ${tests_source})
