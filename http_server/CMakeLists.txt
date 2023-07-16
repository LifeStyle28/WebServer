include_directories(${CMAKE_SOURCE_DIR}/application ${CMAKE_SOURCE_DIR}/logger)

add_library(http_server SHARED
    http_server.cpp
    request_handler.cpp
    response_builder.cpp
    api_handler.cpp
)

target_link_libraries(http_server PUBLIC app)
