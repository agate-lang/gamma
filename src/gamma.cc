/* gamma, GAMes Made with Agate
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2022 Julien Bernard
 */
#include <cstdio>
#include <cstdlib>

#include <SDL2/SDL.h>

#include "agate.h"

#include "gamma_color.h"
#include "gamma_event.h"
#include "gamma_math.h"
#include "gamma_render.h"
#include "gamma_sprite.h"
#include "gamma_support.h"
#include "gamma_time.h"
#include "gamma_window.h"

#include "config.h"

static void usage(void) {
  std::printf("Usage: gamma <unit>\n");
}

static void print(AgateVM *vm, const char* text) {
  std::fputs(text, stdout);
}

static void write(AgateVM *vm, uint8_t byte) {
  std::fputc(byte, stdout);
}

static void error(AgateVM *vm, AgateErrorKind kind, const char *unit_name, int line, const char *message) {
  switch (kind) {
    case AGATE_ERROR_COMPILE:
      std::fprintf(stderr, "%s:%d: error: %s\n", unit_name, line, message);
      break;
    case AGATE_ERROR_RUNTIME:
      std::fprintf(stderr, "error: %s\n", message);
      break;
    case AGATE_ERROR_STACKTRACE:
      std::fprintf(stderr, "%s:%d: in %s\n", unit_name, line, message);
      break;
  }
}

static void input(AgateVM *vm, char *buffer, size_t size) {
  if (std::fgets(buffer, (int) size, stdin) == NULL) {
    buffer[0] = '\0';
  }
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    usage();
    return EXIT_FAILURE;
  }

  // initialize SDL

  if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
    SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
    return EXIT_FAILURE;
  }

  // initialize Agate

  gma::Support support;

  AgateConfig config;
  agateConfigInitialize(&config);

  config.unit_handler = gma::Support::unit_handler;
  config.foreign_class_handler = gma::Support::foreign_class_handler;
  config.foreign_method_handler = gma::Support::foreign_method_handler;

  config.print = print;
  config.write = write;
  config.error = error;
  config.input = input;

  config.user_data = &support;

  AgateVM *vm = agateNewVM(&config);

  support.add_include_path(GAMMA_UNIT_DIRECTORY);

  gma::ColorUnit::provide_support(support);
  gma::EventUnit::provide_support(support);
  gma::MathUnit::provide_support(support);
  gma::RenderUnit::provide_support(support);
  gma::SpriteUnit::provide_support(support);
  gma::TimeUnit::provide_support(support);
  gma::WindowUnit::provide_support(support);

  // run the game

  const char *source = support.load_unit(argv[1]);

  if (source != nullptr) {
    agateCallString(vm, argv[1], source);
  } else {
    fprintf(stderr, "Could not find gamma unit '%s'.\n", argv[1]);
  }

  // shutdown Agate

  agateDeleteVM(vm);

  // shutdown SDL

  SDL_Quit();

  return EXIT_SUCCESS;
}
