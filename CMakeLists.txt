cmake_minimum_required(VERSION 3.7)

project(GAMMA
  VERSION 0.1.0
  LANGUAGES C
)

option(GAMMA_USE_EMBEDDED_LIBS "Build the library with the submodule libraries" ON)

include(GNUInstallDirs)

find_package(SDL2 REQUIRED)

if(GAMMA_USE_EMBEDDED_LIBS)
  message(STATUS "Build with embedded libraries")
  set(Stb_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/vendor/stb")
else()
  # works on vcpkg
  find_package(Stb REQUIRED)
endif()

set(GAMMA_UNIT_DIRECTORY "${CMAKE_INSTALL_FULL_DATAROOTDIR}/gamma/unit")
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/src/config.h.in" "${CMAKE_CURRENT_BINARY_DIR}/config.h" @ONLY)

add_executable(gamma
  src/gamma.c
  src/gamma_check.c
  src/gamma_common.c
  src/gamma_debug.c
  src/gamma_error.c
  src/gamma_event.c
  src/gamma_math.c
  src/gamma_render.c
  src/gamma_sprite.c
  src/gamma_time.c
  src/gamma_utils.c
  src/gamma_window.c

  vendor/agate/agate.c
  vendor/agate/agate-support.c

  vendor/glad/src/glad.c
)

if(MSVC)
  target_compile_options(gamma PRIVATE /W3)
  target_compile_definitions(gamma
    PRIVATE
      NOMINMAX
      _CRT_SECURE_NO_WARNINGS
  )
  target_link_options(gamma
    PRIVATE
      /NODEFAULTLIB:$<IF:$<CONFIG:Debug>,libcmtd,libcmt>
  )
else()
  target_compile_options(gamma PRIVATE -Wall)
endif()

target_include_directories(gamma
  PRIVATE
    "${CMAKE_CURRENT_BINARY_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/vendor/agate"
    "${CMAKE_CURRENT_SOURCE_DIR}/vendor/glad/include"
    "${Stb_INCLUDE_DIR}"
)

target_link_libraries(gamma
  PRIVATE
    $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
    $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
)

if(UNIX)
  target_link_libraries(gamma
    PRIVATE
      m
  )
endif()

install(
  TARGETS gamma
  RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
)

install(
  DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/unit"
  DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/gamma"
)
