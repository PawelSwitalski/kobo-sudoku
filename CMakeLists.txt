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

# --- UI: renderer-agnostic screens and widgets ---
add_library(sudoku_ui STATIC
  src/ui/theme.cpp
  src/ui/widgets.cpp
  src/ui/screens/game_screen.cpp
  src/ui/screens/menu_screen.cpp
  src/ui/screens/stats_screen.cpp
  src/ui/screens/settings_screen.cpp
)
target_link_libraries(sudoku_ui PUBLIC sudoku_core)

if(SUDOKU_BACKEND STREQUAL "sdl")
  find_package(SDL2 CONFIG QUIET)
  if(NOT SDL2_FOUND AND MSVC)
    # Prebuilt VC development package (host simulator only, Constitution VI).
    include(FetchContent)
    FetchContent_Declare(sdl2_prebuilt
      URL https://github.com/libsdl-org/SDL/releases/download/release-2.30.11/SDL2-devel-2.30.11-VC.zip
    )
    FetchContent_MakeAvailable(sdl2_prebuilt)
    set(SDL2_DIR "${sdl2_prebuilt_SOURCE_DIR}/cmake")
    find_package(SDL2 CONFIG REQUIRED)
  elseif(NOT SDL2_FOUND)
    find_package(SDL2 REQUIRED)
  endif()

  add_executable(sudoku
    src/main.cpp
    src/platform/softcanvas.cpp
    src/platform/sdl/sdl_renderer.cpp
    src/platform/sdl/mouse_touch.cpp
  )
  target_compile_definitions(sudoku PRIVATE SUDOKU_BACKEND_SDL SDL_MAIN_HANDLED)
  target_link_libraries(sudoku PRIVATE sudoku_ui sudoku_persist SDL2::SDL2)
  if(TARGET SDL2::SDL2 AND MSVC)
    add_custom_command(TARGET sudoku POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_if_different
              $<TARGET_FILE:SDL2::SDL2> $<TARGET_FILE_DIR:sudoku>)
  endif()
elseif(SUDOKU_BACKEND STREQUAL "fbink")
  # Cross build (cmake/kobo-toolchain.cmake). FBInk is built by
  # tools/build-fbink.sh into third_party/FBInk/Release/.
  set(FBINK_LIB "${CMAKE_SOURCE_DIR}/third_party/FBInk/Release/libfbink.a")
  if(NOT EXISTS "${FBINK_LIB}")
    message(FATAL_ERROR "FBInk not built. Run tools/build-fbink.sh first (see quickstart.md).")
  endif()

  # FBInk's KOBO build resolves I2C (e.g. query_accelerometer) against its own
  # vendored/staged libi2c.a rather than bundling those symbols into
  # libfbink.a itself, so consumers must link it too.
  set(FBINK_I2C_LIB "${CMAKE_SOURCE_DIR}/third_party/FBInk/libi2c-staged/lib/libi2c.a")
  if(NOT EXISTS "${FBINK_I2C_LIB}")
    message(FATAL_ERROR "FBInk's staged libi2c.a not found. Run tools/build-fbink.sh first (see quickstart.md).")
  endif()

  add_executable(sudoku
    src/main.cpp
    src/platform/softcanvas.cpp
    src/platform/kobo/fbink_renderer.cpp
    src/platform/kobo/evdev_touch.cpp
  )
  target_compile_definitions(sudoku PRIVATE SUDOKU_BACKEND_FBINK)
  target_include_directories(sudoku PRIVATE third_party/FBInk)
  target_link_libraries(sudoku PRIVATE sudoku_ui sudoku_persist "${FBINK_LIB}" "${FBINK_I2C_LIB}" m rt pthread)
endif()

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
