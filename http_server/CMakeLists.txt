include_directories(${CMAKE_SOURCE_DIR}/logger)

add_library(http_server SHARED
    http_server.cpp
    request_handler.cpp
    response_builder.cpp
    api_handler.cpp
)

target_include_directories(http_server PUBLIC ${CMAKE_SOURCE_DIR}/application)
target_link_libraries(http_server PUBLIC app CONAN_PKG::boost)
