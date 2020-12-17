cmake_minimum_required(VERSION 3.17)
project(flat)

set(CMAKE_CXX_STANDARD 17)

add_executable(flat main.cpp main.cpp symbol.h grammar.h earley_parser.h solver.h)

enable_testing()
add_executable(tests tests.cpp)
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})
target_link_libraries(tests ${GTEST_BOTH_LIBRARIES} ${GTEST_LIBRARIES} pthread)