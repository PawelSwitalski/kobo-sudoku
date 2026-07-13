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

# --- Portable core: pure logic, no OS calls (Constitution I) ---
add_library(sudoku_core STATIC
  src/core/board.cpp
  src/core/solver.cpp
  src/core/difficulty.cpp
  src/core/generator.cpp
  src/core/session.cpp
  src/core/stats.cpp
)
target_include_directories(sudoku_core PUBLIC src third_party)

# --- Persistence: atomic JSON files (Constitution V) ---
add_library(sudoku_persist STATIC
  src/persist/paths.cpp
  src/persist/store.cpp
)
target_link_libraries(sudoku_persist PUBLIC sudoku_core)

if(BUILD_TESTS)
  enable_testing()
  add_executable(sudoku_tests
    tests/main.cpp
    tests/test_smoke.cpp
    tests/test_rules.cpp
    tests/test_pencilmarks.cpp
    tests/test_generator.cpp
    tests/test_difficulty.cpp
    tests/test_persist.cpp
  )
  target_link_libraries(sudoku_tests PRIVATE sudoku_core sudoku_persist)
  add_test(NAME core COMMAND sudoku_tests)
endif()
