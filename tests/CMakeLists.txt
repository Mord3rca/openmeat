enable_testing()

project(${GLOBAL_NAME}tests)

FILE(GLOB tests_source *.cpp)

add_executable(${PROJECT_NAME})

pkg_search_module(CPPUNIT REQUIRED IMPORTED_TARGET cppunit)

target_link_libraries(${PROJECT_NAME} ${GLOBAL_NAME}_shared PkgConfig::CPPUNIT)

add_compile_options(
    -Wall
    $<$<CONFIG:DEBUG>:-g>
)

target_sources(${PROJECT_NAME} PRIVATE ${tests_source})

add_test(NAME ${PROJECT_NAME} COMMAND ./${PROJECT_NAME})
