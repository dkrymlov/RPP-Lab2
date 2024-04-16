cmake_minimum_required(VERSION 3.26.4)
project(LABA2)

set(CMAKE_CXX_STANDARD 17)
find_package(MPI REQUIRED)
add_executable(LABA2 main.cpp)
target_link_libraries(LABA2 PUBLIC MPI::MPI_CXX)
