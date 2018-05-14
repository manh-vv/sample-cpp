cmake_minimum_required(VERSION 3.10)
project(clion)

set(CMAKE_CXX_STANDARD 14)

set(CURL_LIBRARY "-lcurl")
find_package(CURL REQUIRED)


add_executable(clion main.cpp)

include_directories(${CURL_INCLUDE_DIR})
target_link_libraries(clion ${CURL_LIBRARIES})