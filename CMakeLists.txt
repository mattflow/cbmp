cmake_minimum_required(VERSION 3.2)

project(cbmp)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_BUILD_TYPE DEBUG)

set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Wextra")

add_subdirectory(lib/googletest)

include_directories(
  ${GTEST_ROOT}/include
)

add_executable(
  tests
  tests.cc
  cbmp.c
)

target_link_libraries(
  tests
  gtest_main
)

enable_testing()
add_test(CBMPTests tests)
