cmake_minimum_required(VERSION 3.7)

project(GAMMA
  VERSION 0.1.0
  LANGUAGES C
)

add_executable(gamma
  src/gamma.c
)

if(MSVC)
  target_compile_options(gamma PRIVATE /W3)
else()
  target_compile_options(gamma PRIVATE -Wall -Wpedantic)
endif()

target_include_directories(gamma
  PRIVATE
    "${CMAKE_CURRENT_BINARY_DIR}"
)

target_link_libraries(gamma
  PRIVATE
    m
)

install(
  TARGETS gamma
  RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
)
