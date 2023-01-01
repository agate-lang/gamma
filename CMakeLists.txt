cmake_minimum_required(VERSION 3.7)

project(GAMMA
  VERSION 0.1.0
  LANGUAGES C CXX
)

option(GAMMA_USE_EMBEDDED_LIBS "Build the library with the submodule libraries" ON)

include(GNUInstallDirs)

find_package(SDL2 REQUIRED)
find_package(Freetype REQUIRED)

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
  src/gamma.cc
  src/gamma_agate.cc
  src/gamma_color.cc
  src/gamma_debug.cc
  src/gamma_event.cc
  src/gamma_math.cc
  src/gamma_render.cc
  src/gamma_sprite.cc
  src/gamma_support.cc
  src/gamma_tag.cc
  src/gamma_text.cc
  src/gamma_time.cc
  src/gamma_window.cc

  vendor/agate/agate.c
  vendor/glad/src/glad.c
)

target_compile_features(gamma
  PUBLIC
    cxx_std_17
    c_std_99
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
    Freetype::Freetype
)

install(
  TARGETS gamma
  RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
)

install(
  DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/unit"
  DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/gamma"
)
