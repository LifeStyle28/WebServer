cmake_minimum_required(VERSION 3.11)

project(web_server CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wformat -Wformat-security -Werror")

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)

find_package(Boost 1.78.0 REQUIRED)
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
endif()

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)

# boost.beast будет использовать std::string_view вместо boost::string_view
add_definitions(-DBOOST_BEAST_USE_STD_STRING_VIEW)

add_subdirectory(application)
add_subdirectory(http_server)
add_subdirectory(logger)

include_directories(
    http_server
    logger
    application
)

add_executable(web_server
    main.cpp
)

target_link_libraries(web_server http_server app log)
