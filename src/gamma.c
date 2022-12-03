/* gamma, GAMes Made with Agate
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2022 Julien Bernard
 */
#include <stdlib.h>

#include <SDL2/SDL.h>

#include "agate.h"
#include "agate-support.h"

#include "gamma_common.h"
#include "gamma_event.h"
#include "gamma_gfx.h"
#include "gamma_root.h"
#include "gamma_time.h"
#include "gamma_window.h"

#include "config.h"

static void usage(void) {
  printf("Usage: gamma <unit>\n");
}

static void print(AgateVM *vm, const char* text) {
  fputs(text, stdout);
}

static void write(AgateVM *vm, uint8_t byte) {
  fputc(byte, stdout);
}

static void error(AgateVM *vm, AgateErrorKind kind, const char *unit_name, int line, const char *message) {
  switch (kind) {
    case AGATE_ERROR_COMPILE:
      fprintf(stderr, "%s:%d: error: %s\n", unit_name, line, message);
      break;
    case AGATE_ERROR_RUNTIME:
      fprintf(stderr, "error: %s\n", message);
      break;
    case AGATE_ERROR_STACKTRACE:
      fprintf(stderr, "%s:%d: in %s\n", unit_name, line, message);
      break;
  }
}

static void input(AgateVM *vm, char *buffer, size_t size) {
  fgets(buffer, (int) size, stdin);
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

  AgateConfig config;
  agateConfigInitialize(&config);

  config.unit_handler = agateExUnitHandler;
  config.foreign_class_handler = agateExForeignClassHandler;
  config.foreign_method_handler = agateExForeignMethodHandler;

  config.print = print;
  config.write = write;
  config.error = error;
  config.input = input;

  AgateVM *vm = agateExNewVM(&config);

  agateExUnitAddIncludePath(vm, GAMMA_UNIT_DIRECTORY);

  agateExForeignClassAddHandler(vm, gammaRootClassHandler, "gamma");
  agateExForeignClassAddHandler(vm, gammaEventClassHandler, "gamma/event");
  agateExForeignClassAddHandler(vm, gammaGfxClassHandler, "gamma/gfx");
  agateExForeignClassAddHandler(vm, gammaWindowClassHandler, "gamma/window");


  agateExForeignMethodAddHandler(vm, gammaRootMethodHandler, "gamma");
  agateExForeignMethodAddHandler(vm, gammaEventMethodHandler, "gamma/event");
  agateExForeignMethodAddHandler(vm, gammaGfxMethodHandler, "gamma/gfx");
  agateExForeignMethodAddHandler(vm, gammaTimeMethodHandler, "gamma/time");
  agateExForeignMethodAddHandler(vm, gammaWindowMethodHandler, "gamma/window");

  // run the game

  const char *source = agateExUnitLoad(vm, argv[1]);

  if (source != NULL) {
    AgateStatus status = agateCallString(vm, argv[1], source);
    agateExUnitRelease(vm, source);

    if (status != AGATE_STATUS_OK) {
      fprintf(stderr, "Error in the gamma unit '%s'.\n", argv[1]);
    }
  } else {
    fprintf(stderr, "Could not find gamma unit '%s'.\n", argv[1]);
  }

  // shutdown Agate

  agateExDeleteVM(vm);

  // shutdown SDL

  SDL_Quit();

  return EXIT_SUCCESS;
}
