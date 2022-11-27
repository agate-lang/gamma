#include "gamma_gfx.h"

#include <assert.h>

#include <SDL2/SDL.h>

#include "glad/glad.h"

#include "gamma_common.h"
#include "gamma_root.h"
#include "gamma_tags.h"
#include "gamma_window.h"

struct GammaRenderer {
  SDL_GLContext context;
  GLuint vao;
};

// class

static ptrdiff_t gammaRendererAllocate(AgateVM *vm, const char *unit_name, const char *class_name) {
  return sizeof(struct GammaRenderer);
}

static uint64_t gammaRendererTag(AgateVM *vm, const char *unit_name, const char *class_name) {
  return GAMMA_RENDERER_TAG;
}

static void gammaRendererDestroy(AgateVM *vm, const char *unit_name, const char *class_name, void *data) {
  struct GammaRenderer *renderer = data;

  if (renderer->context != NULL) {
    glBindVertexArray(0);
    glDeleteVertexArrays(1, &renderer->vao);
    SDL_GL_DeleteContext(renderer->context); // TODO: error
    renderer->context = NULL;
  }
}

// methods

static void gammaRendererNew(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_RENDERER_TAG);
  struct GammaRenderer *renderer = agateSlotGetForeign(vm, 0);
  assert(agateSlotGetForeignTag(vm, 1) == GAMMA_WINDOW_TAG); // TODO: error
  struct GammaWindow *window = agateSlotGetForeign(vm, 1);

  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

  renderer->context = SDL_GL_CreateContext(window->ptr); // TODO: error

  SDL_GL_MakeCurrent(window->ptr, renderer->context); // TODO: error
  gladLoadGLLoader(SDL_GL_GetProcAddress); // TODO: error

  glEnable(GL_BLEND);
  glEnable(GL_SCISSOR_TEST);
  glEnable(GL_COLOR_BUFFER_BIT);

  glGenVertexArrays(1, &renderer->vao);
  glBindVertexArray(renderer->vao);
}

static void gammaRendererClear0(AgateVM *vm) {
  glClear(GL_COLOR_BUFFER_BIT);
}

static void gammaRendererClear1(AgateVM *vm) {
  glClearColor(1.0, 1.0, 1.0, 1.0);
  glClear(GL_COLOR_BUFFER_BIT);
}

static void gammaRendererDisplay(AgateVM *vm) {
  SDL_GL_SwapWindow(SDL_GL_GetCurrentWindow());
}

/*
 * unit configuration
 */

AgateForeignClassHandler gammaGfxClassHandler(AgateVM *vm, const char *unit_name, const char *class_name) {
  assert(gammaEquals(unit_name, "gamma/gfx"));
  AgateForeignClassHandler handler = { NULL, NULL, NULL };

  if (gammaEquals(class_name, "Renderer")) {
    handler.allocate = gammaRendererAllocate;
    handler.tag = gammaRendererTag;
    handler.destroy = gammaRendererDestroy;
    return handler;
  }

  return handler;
}

AgateForeignMethodFunc gammaGfxMethodHandler(AgateVM *vm, const char *unit_name, const char *class_name, AgateForeignMethodKind kind, const char *signature) {
  assert(gammaEquals(unit_name, "gamma/gfx"));

  if (gammaEquals(class_name, "Renderer")) {
    if (kind == AGATE_FOREIGN_METHOD_INSTANCE) {
      if (gammaEquals(signature, "init new(_)")) { return gammaRendererNew; }
      if (gammaEquals(signature, "clear()")) { return gammaRendererClear0; }
      if (gammaEquals(signature, "clear(_)")) { return gammaRendererClear1; }
      if (gammaEquals(signature, "display()")) { return gammaRendererDisplay; }
    }
  }

  return NULL;
}
