cmake_minimum_required(VERSION 3.7)

project(GAMMA
  VERSION 0.1.0
  LANGUAGES C
)

find_package(sdl2 REQUIRED)

add_executable(gamma
  src/gamma.c
  src/gamma_common.c
  src/gamma_time.c

  vendor/agate/agate.c
  vendor/agate/agate-support.c
)

if(MSVC)
  target_compile_options(gamma PRIVATE /W3)
else()
  target_compile_options(gamma PRIVATE -Wall -Wpedantic)
endif()

target_include_directories(gamma
  PRIVATE
    "${CMAKE_CURRENT_BINARY_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/vendor/agate"
)

target_link_libraries(gamma
  PRIVATE
    SDL2::SDL2
    m
)

install(
  TARGETS gamma
  RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
)
