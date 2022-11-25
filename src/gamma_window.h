#ifndef GAMMA_WINDOW_H
#define GAMMA_WINDOW_H

#include <SDL2/SDL.h>

#include "agate.h"

struct gammaWindow {
  SDL_Window *ptr;
  bool should_close;
  bool fullscreen;
};

AgateForeignClassHandler gammaWindowClassHandler(AgateVM *vm, const char *unit_name, const char *class_name);
AgateForeignMethodFunc gammaWindowMethodHandler(AgateVM *vm, const char *unit_name, const char *class_name, AgateForeignMethodKind kind, const char *signature);


#endif // GAMMA_WINDOW_H
