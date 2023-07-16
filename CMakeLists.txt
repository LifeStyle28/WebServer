cmake_minimum_required(VERSION 3.11)

project(web_server CXX)
set(CMAKE_CXX_STANDARD 20)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)

find_package(Boost 1.78.0 REQUIRED)
if(Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
endif()

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# boost.beast будет использовать std::string_view вместо boost::string_view
add_definitions(-DBOOST_BEAST_USE_STD_STRING_VIEW)

add_subdirectory(application)
add_subdirectory(http_server)

include_directories(
    http_server
    logger
)

add_executable(web_server
    main.cpp
    logger/boost_logger.cpp
)

target_link_libraries(web_server http_server)
