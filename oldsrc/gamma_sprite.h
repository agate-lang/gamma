#ifndef GAMMA_SPRITE_H
#define GAMMA_SPRITE_H

#include "agate.h"

#include "glad/glad.h"

#include "gamma_render.h"
#include "gamma_math.h"

enum GammaTextureKind {
  GAMMA_TEXTURE_COLOR,
  GAMMA_TEXTURE_ALPHA,
};

GLuint gammaTextureRawCreate(GLsizei width, GLsizei height, enum GammaTextureKind kind, const uint8_t *data);

void gammaSpriteRawRender(AgateVM *vm, ptrdiff_t slot, struct GammaRendererData *data);

AgateForeignClassHandler gammaSpriteClassHandler(AgateVM *vm, const char *unit_name, const char *class_name);
AgateForeignMethodFunc gammaSpriteMethodHandler(AgateVM *vm, const char *unit_name, const char *class_name, AgateForeignMethodKind kind, const char *signature);

#endif // GAMMA_SPRITE_H
