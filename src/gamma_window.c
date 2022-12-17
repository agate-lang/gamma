#include "gamma_window.h"

#include <assert.h>
#include <stdbool.h>

#include "gamma_common.h"
#include "gamma_check.h"
#include "gamma_error.h"
#include "gamma_root.h"
#include "gamma_tags.h"

/*
 * Window
 */

// raw

static void gammaWindowRawSetFullscreen(struct GammaWindow *window, bool fullscreen, AgateVM *vm) {
  if (fullscreen) {
    if (SDL_SetWindowFullscreen(window->ptr, SDL_WINDOW_FULLSCREEN_DESKTOP) != 0) {
      gammaError(vm, "Unable to set the window fullscreen.");
    }
  } else {
    if (SDL_SetWindowFullscreen(window->ptr, 0) != 0) {
      gammaError(vm, "Unable to come back in windowed mode.");
    }
  }

  window->fullscreen = fullscreen;
}

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

  if (window->ptr == NULL) {
    gammaError(vm, "Unable to create a window: %s\n", SDL_GetError());
    return;
  }

  window->should_close = false;
  window->fullscreen = false;
}

static void gammaWindowOpen(AgateVM *vm) {
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

static void gammaWindowGetId(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_WINDOW_TAG);
  struct GammaWindow *window = agateSlotGetForeign(vm, 0);
  Uint32 id = SDL_GetWindowID(window->ptr);

  if (id == 0) {
    gammaError(vm, "Unable to get window id.");
    agateSlotSetNil(vm, AGATE_RETURN_SLOT);
    return;
  }

  agateSlotSetInt(vm, AGATE_RETURN_SLOT, id);
}

static void gammaWindowGetTitle(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_WINDOW_TAG);
  struct GammaWindow *window = agateSlotGetForeign(vm, 0);
  agateSlotSetString(vm, AGATE_RETURN_SLOT, SDL_GetWindowTitle(window->ptr));
}

static void gammaWindowSetTitle(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_WINDOW_TAG);
  struct GammaWindow *window = agateSlotGetForeign(vm, 0);

  const char *title;

  if (!gammaCheckString(vm, 1, &title)) {
    gammaError(vm, "String parameter expected for `title`.");
    agateSlotSetNil(vm, AGATE_RETURN_SLOT);
    return;
  }

  SDL_SetWindowTitle(window->ptr, title);
  agateSlotCopy(vm, AGATE_RETURN_SLOT, 1);
}

static void gammaWindowGetPosition(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_WINDOW_TAG);
  struct GammaWindow *window = agateSlotGetForeign(vm, 0);

  ptrdiff_t class_slot = agateSlotAllocate(vm);
  agateGetVariable(vm, "gamma", "Vec2I", class_slot);
  struct GammaVec2I *result = agateSlotSetForeign(vm, AGATE_RETURN_SLOT, class_slot);

  SDL_GetWindowPosition(window->ptr, &result->v[0], &result->v[1]);
}

static void gammaWindowSetPosition(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_WINDOW_TAG);
  struct GammaWindow *window = agateSlotGetForeign(vm, 0);

  struct GammaVec2I size;

  if (!gammaCheckVec2I(vm, 1, &size)) {
    gammaError(vm, "Vec2I parameter expected for `value`.");
    agateSlotSetNil(vm, AGATE_RETURN_SLOT);
    return;
  }

  SDL_SetWindowPosition(window->ptr, size.v[0], size.v[1]);
  agateSlotCopy(vm, AGATE_RETURN_SLOT, 1);
}

static void gammaWindowGetSize(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_WINDOW_TAG);
  struct GammaWindow *window = agateSlotGetForeign(vm, 0);

  ptrdiff_t class_slot = agateSlotAllocate(vm);
  agateGetVariable(vm, "gamma", "Vec2I", class_slot);
  struct GammaVec2I *result = agateSlotSetForeign(vm, AGATE_RETURN_SLOT, class_slot);

  SDL_GetWindowSize(window->ptr, &result->v[0], &result->v[1]);
}

static void gammaWindowSetSize(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_WINDOW_TAG);
  struct GammaWindow *window = agateSlotGetForeign(vm, 0);

  struct GammaVec2I size;

  if (!gammaCheckVec2I(vm, 1, &size)) {
    gammaError(vm, "Vec2I parameter expected for `value`.");
    agateSlotSetNil(vm, AGATE_RETURN_SLOT);
    return;
  }

  SDL_SetWindowSize(window->ptr, size.v[0], size.v[1]);
  agateSlotCopy(vm, AGATE_RETURN_SLOT, 1);
}

static void gammaWindowGetFramebufferSize(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_WINDOW_TAG);
  struct GammaWindow *window = agateSlotGetForeign(vm, 0);

  ptrdiff_t class_slot = agateSlotAllocate(vm);
  agateGetVariable(vm, "gamma", "Vec2I", class_slot);
  struct GammaVec2I *result = agateSlotSetForeign(vm, AGATE_RETURN_SLOT, class_slot);

  SDL_GL_GetDrawableSize(window->ptr, &result->v[0], &result->v[1]);
}

static void gammaWindowIsFullscreen(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_WINDOW_TAG);
  struct GammaWindow *window = agateSlotGetForeign(vm, 0);
  agateSlotSetBool(vm, AGATE_RETURN_SLOT, window->fullscreen);
}

static void gammaWindowSetFullscreen(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_WINDOW_TAG);
  struct GammaWindow *window = agateSlotGetForeign(vm, 0);

  bool fullscreen;

  if (!gammaCheckBool(vm, 1, &fullscreen)) {
    gammaError(vm, "Bool parameter expected for `value`.");
    agateSlotSetNil(vm, AGATE_RETURN_SLOT);
    return;
  }

  gammaWindowRawSetFullscreen(window, fullscreen, vm);
  agateSlotCopy(vm, AGATE_RETURN_SLOT, 1);
}

static void gammaWindowToggleFullscreen(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_WINDOW_TAG);
  struct GammaWindow *window = agateSlotGetForeign(vm, 0);

  gammaWindowRawSetFullscreen(window, !window->fullscreen, vm);
  agateSlotSetNil(vm, AGATE_RETURN_SLOT);
}

static void gammaWindowIsMinimized(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_WINDOW_TAG);
  struct GammaWindow *window = agateSlotGetForeign(vm, 0);

  Uint32 flags = SDL_GetWindowFlags(window->ptr);
  agateSlotSetBool(vm, AGATE_RETURN_SLOT, (flags & SDL_WINDOW_MINIMIZED) != 0);
}

static void gammaWindowMinimize(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_WINDOW_TAG);
  struct GammaWindow *window = agateSlotGetForeign(vm, 0);

  SDL_MinimizeWindow(window->ptr);
  agateSlotSetNil(vm, AGATE_RETURN_SLOT);
}

static void gammaWindowIsMaximized(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_WINDOW_TAG);
  struct GammaWindow *window = agateSlotGetForeign(vm, 0);

  Uint32 flags = SDL_GetWindowFlags(window->ptr);
  agateSlotSetBool(vm, AGATE_RETURN_SLOT, (flags & SDL_WINDOW_MAXIMIZED) != 0);
}

static void gammaWindowMaximize(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_WINDOW_TAG);
  struct GammaWindow *window = agateSlotGetForeign(vm, 0);

  SDL_MaximizeWindow(window->ptr);
  agateSlotSetNil(vm, AGATE_RETURN_SLOT);
}

static void gammaWindowRestore(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_WINDOW_TAG);
  struct GammaWindow *window = agateSlotGetForeign(vm, 0);

  SDL_RestoreWindow(window->ptr);
  agateSlotSetNil(vm, AGATE_RETURN_SLOT);
}

static void gammaWindowIsVisible(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_WINDOW_TAG);
  struct GammaWindow *window = agateSlotGetForeign(vm, 0);

  Uint32 flags = SDL_GetWindowFlags(window->ptr);
  agateSlotSetBool(vm, AGATE_RETURN_SLOT, (flags & SDL_WINDOW_SHOWN) != 0);
}

static void gammaWindowSetVisible(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_WINDOW_TAG);
  struct GammaWindow *window = agateSlotGetForeign(vm, 0);

  bool visible;

  if (!gammaCheckBool(vm, 1, &visible)) {
    gammaError(vm, "Bool parameter expected for `value`.");
    agateSlotSetNil(vm, AGATE_RETURN_SLOT);
    return;
  }

  if (visible) {
    SDL_ShowWindow(window->ptr);
  } else {
    SDL_HideWindow(window->ptr);
  }

  agateSlotCopy(vm, AGATE_RETURN_SLOT, 1);
}

static void gammaWindowIsDecorated(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_WINDOW_TAG);
  struct GammaWindow *window = agateSlotGetForeign(vm, 0);

  Uint32 flags = SDL_GetWindowFlags(window->ptr);
  agateSlotSetBool(vm, AGATE_RETURN_SLOT, (flags & SDL_WINDOW_BORDERLESS) == 0);
}

static void gammaWindowSetDecorated(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_WINDOW_TAG);
  struct GammaWindow *window = agateSlotGetForeign(vm, 0);

  bool decorated;

  if (!gammaCheckBool(vm, 1, &decorated)) {
    gammaError(vm, "Bool parameter expected for `value`.");
    agateSlotSetNil(vm, AGATE_RETURN_SLOT);
    return;
  }

  SDL_SetWindowBordered(window->ptr, decorated ? SDL_TRUE : SDL_FALSE);
  agateSlotCopy(vm, AGATE_RETURN_SLOT, 1);
}

static void gammaWindowIsResizable(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_WINDOW_TAG);
  struct GammaWindow *window = agateSlotGetForeign(vm, 0);

  Uint32 flags = SDL_GetWindowFlags(window->ptr);
  agateSlotSetBool(vm, AGATE_RETURN_SLOT, (flags & SDL_WINDOW_RESIZABLE) != 0);
}

static void gammaWindowSetResizable(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_WINDOW_TAG);
  struct GammaWindow *window = agateSlotGetForeign(vm, 0);

  bool resizable;

  if (!gammaCheckBool(vm, 1, &resizable)) {
    gammaError(vm, "Bool parameter expected for `value`.");
    agateSlotSetNil(vm, AGATE_RETURN_SLOT);
    return;
  }

  SDL_SetWindowResizable(window->ptr, resizable ? SDL_TRUE : SDL_FALSE);
  agateSlotCopy(vm, AGATE_RETURN_SLOT, 1);
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
      if (gammaEquals(signature, "open")) { return gammaWindowOpen; }
      if (gammaEquals(signature, "close()")) { return gammaWindowClose; }
      if (gammaEquals(signature, "id")) { return gammaWindowGetId; }
      if (gammaEquals(signature, "title")) { return gammaWindowGetTitle; }
      if (gammaEquals(signature, "title=(_)")) { return gammaWindowSetTitle; }
      if (gammaEquals(signature, "position")) { return gammaWindowGetPosition; }
      if (gammaEquals(signature, "position=(_)")) { return gammaWindowSetPosition; }
      if (gammaEquals(signature, "size")) { return gammaWindowGetSize; }
      if (gammaEquals(signature, "size=(_)")) { return gammaWindowSetSize; }
      if (gammaEquals(signature, "framebuffer_size")) { return gammaWindowGetFramebufferSize; }
      if (gammaEquals(signature, "fullscreen")) { return gammaWindowIsFullscreen; }
      if (gammaEquals(signature, "fullscreen=(_)")) { return gammaWindowSetFullscreen; }
      if (gammaEquals(signature, "toggle_fullscreen()")) { return gammaWindowToggleFullscreen; }
      if (gammaEquals(signature, "minimized")) { return gammaWindowIsMinimized; }
      if (gammaEquals(signature, "minimize()")) { return gammaWindowMinimize; }
      if (gammaEquals(signature, "maximized")) { return gammaWindowIsMaximized; }
      if (gammaEquals(signature, "maximize()")) { return gammaWindowMaximize; }
      if (gammaEquals(signature, "restore()")) { return gammaWindowRestore; }
      if (gammaEquals(signature, "visible")) { return gammaWindowIsVisible; }
      if (gammaEquals(signature, "visible=(_)")) { return gammaWindowSetVisible; }
      if (gammaEquals(signature, "decorated")) { return gammaWindowIsDecorated; }
      if (gammaEquals(signature, "decorated=(_)")) { return gammaWindowSetDecorated; }
      if (gammaEquals(signature, "resizable")) { return gammaWindowIsResizable; }
      if (gammaEquals(signature, "resizable=(_)")) { return gammaWindowSetResizable; }
    }
  }

  return NULL;
}
