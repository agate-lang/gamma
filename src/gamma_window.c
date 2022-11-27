#include "gamma_window.h"

#include <assert.h>
#include <stdbool.h>

#include "gamma_common.h"
#include "gamma_tags.h"

/*
 * Window
 */

// class

static ptrdiff_t gammaWindowAllocate(AgateVM *vm, const char *unit_name, const char *class_name) {
  return sizeof(struct GammaWindow);
}

static uint64_t gammaWindowTag(AgateVM *vm, const char *unit_name, const char *class_name) {
  return GAMMA_WINDOW_TAG;
}

static void gammaWindowDestroy(AgateVM *vm, const char *unit_name, const char *class_name, void *data) {
  struct GammaWindow *window = data;

  if (window->ptr != NULL) {
    SDL_DestroyWindow(window->ptr);
    window->ptr = NULL;
  }
}

// methods

static void gammaWindowNew(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_WINDOW_TAG);
  struct GammaWindow *window = agateSlotGetForeign(vm, 0);


  const char *title = agateSlotGetString(vm, 1);
  int w = (int) agateSlotGetInt(vm, 2);
  int h = (int) agateSlotGetInt(vm, 3);

  window->ptr = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, w, h, SDL_WINDOW_OPENGL);
  window->should_close = false;
  window->fullscreen = false;
}

static void gammaWindowIsOpen(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_WINDOW_TAG);
  struct GammaWindow *window = agateSlotGetForeign(vm, 0);
  agateSlotSetBool(vm, AGATE_RETURN_SLOT, !window->should_close);
}

static void gammaWindowClose(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_WINDOW_TAG);
  struct GammaWindow *window = agateSlotGetForeign(vm, 0);
  window->should_close = true;
  agateSlotSetNil(vm, AGATE_RETURN_SLOT);
}

/*
 * unit configuration
 */

AgateForeignClassHandler gammaWindowClassHandler(AgateVM *vm, const char *unit_name, const char *class_name) {
  assert(gammaEquals(unit_name, "gamma/window"));
  AgateForeignClassHandler handler = { NULL, NULL, NULL };

  if (gammaEquals(class_name, "Window")) {
    handler.allocate = gammaWindowAllocate;
    handler.tag = gammaWindowTag;
    handler.destroy = gammaWindowDestroy;
    return handler;
  }

  return handler;
}

AgateForeignMethodFunc gammaWindowMethodHandler(AgateVM *vm, const char *unit_name, const char *class_name, AgateForeignMethodKind kind, const char *signature) {
  assert(gammaEquals(unit_name, "gamma/window"));

  if (gammaEquals(class_name, "Window")) {
    if (kind == AGATE_FOREIGN_METHOD_INSTANCE) {
      if (gammaEquals(signature, "init new(_,_,_)")) { return gammaWindowNew; }
      if (gammaEquals(signature, "is_open")) { return gammaWindowIsOpen; }
      if (gammaEquals(signature, "close()")) { return gammaWindowClose; }
    }
  }

  return NULL;
}
