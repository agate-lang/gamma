/* gamma, GAMes Made with Agate
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2022 Julien Bernard
 */
#include <stdlib.h>

#include <SDL2/SDL.h>

#include "agate.h"
#include "agate-support.h"

int main(int argc, char *argv[]) {

  // initialize SDL

  if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
    SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
    return EXIT_FAILURE;
  }

  // initialize Agate

  AgateConfig config;
  agateConfigInitialize(&config);

  AgateVM *vm = agateExNewVM(&config);


  // shutdown Agate

  agateExDeleteVM(vm);

  // shutdown SDL

  SDL_Quit();

  return EXIT_SUCCESS;
}
