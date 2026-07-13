cmake_minimum_required(VERSION 3.20)
project(kobo_sudoku CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Build flavors (see specs/001-kobo-sudoku/plan.md):
#   -DSUDOKU_BACKEND=none   host: core + tests only
#   -DSUDOKU_BACKEND=sdl    host: desktop simulator
#   -DSUDOKU_BACKEND=fbink  device: Kobo e-ink build (cross, cmake/kobo-toolchain.cmake)
set(SUDOKU_BACKEND "none" CACHE STRING "Rendering backend: none | sdl | fbink")
option(BUILD_TESTS "Build host unit tests" OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release)
endif()

if(BUILD_TESTS)
  enable_testing()
  add_executable(sudoku_tests
    tests/main.cpp
    tests/test_smoke.cpp
  )
  target_include_directories(sudoku_tests PRIVATE src third_party)
  add_test(NAME core COMMAND sudoku_tests)
endif()
