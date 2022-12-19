#ifndef GAMMA_SPRITE_H
#define GAMMA_SPRITE_H

#include "agate.h"

#include "glad/glad.h"

enum GammaTextureFormat {
  GAMMA_TEXTURE_ALPHA,
  GAMMA_TEXTURE_COLOR,
};

#define GAMMA_TEXTURE_SMOOTH    0x01
#define GAMMA_TEXTURE_REPEATED  0x02
#define GAMMA_TEXTURE_MIPMAP    0x04

struct GammaTexture {
  enum GammaTextureFormat format;
  GLuint texture;
  GLsizei width;
  GLsizei height;
  uint32_t flags;
};

GLuint gammaTextureRawCreate(GLsizei width, GLsizei height, GLenum format, GLint alignment, const uint8_t *data);


AgateForeignClassHandler gammaSpriteClassHandler(AgateVM *vm, const char *unit_name, const char *class_name);
AgateForeignMethodFunc gammaSpriteMethodHandler(AgateVM *vm, const char *unit_name, const char *class_name, AgateForeignMethodKind kind, const char *signature);

#endif // GAMMA_SPRITE_H
