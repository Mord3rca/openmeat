FILE(GLOB key_breaker_src *.cpp)

project(${GLOBAL_NAME}_key_breaker)

pkg_search_module(PCAP REQUIRED IMPORTED_TARGET libpcap)

add_executable(${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME} ${GLOBAL_NAME} PkgConfig::PCAP)

add_compile_options(
    -Wall
    -03
    $<$<CONFIG:DEBUG>:-g>
)

target_sources(${PROJECT_NAME} PRIVATE ${key_breaker_src})
