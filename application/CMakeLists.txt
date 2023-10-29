include_directories(${CMAKE_SOURCE_DIR}/logger)

add_library(app SHARED
    application.cpp
    model.cpp
    use_cases.cpp
    json_loader.cpp
    connection.cpp
)

target_link_libraries(app CONAN_PKG::boost)
