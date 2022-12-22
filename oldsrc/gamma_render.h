#ifndef GAMMA_RENDER_H
#define GAMMA_RENDER_H

#include "agate.h"
#include "glad/glad.h"

#include "gamma_math.h"

struct GammaColor {
  float r;
  float g;
  float b;
  float a;
};

void gammaColorRawFromRgba(struct GammaColor *color, int64_t rgba);

enum GammaRendererMode {
  GAMMA_RENDERER_COLOR,
  GAMMA_RENDERER_ALPHA,
};

struct GammaVertex {
  struct GammaVec2F position;
  struct GammaColor color;
  struct GammaVec2F texcoords;
};

struct GammaRendererData {
  GLenum primitive;
  GLsizei count;
  GLuint vertex_buffer;
  GLuint element_buffer;
  enum GammaRendererMode mode;
  GLuint texture0;
  GLuint texture1;
  GLuint shader;
  struct GammaRectF bounds;
  struct GammaMat3F transform;
};

AgateForeignClassHandler gammaRenderClassHandler(AgateVM *vm, const char *unit_name, const char *class_name);
AgateForeignMethodFunc gammaRenderMethodHandler(AgateVM *vm, const char *unit_name, const char *class_name, AgateForeignMethodKind kind, const char *signature);

#endif // GAMMA_RENDER_H
