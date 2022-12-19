#include "gamma_gfx.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#include <SDL2/SDL.h>

#include "glad/glad.h"

#include "gamma_common.h"
#include "gamma_check.h"
#include "gamma_error.h"
#include "gamma_root.h"
#include "gamma_sprite.h"
#include "gamma_tags.h"
#include "gamma_utils.h"
#include "gamma_window.h"

#include "shaders/default.vert.h"
#include "shaders/default.frag.h"
#include "shaders/default_alpha.frag.h"


/*
 * Shader
 */

struct GammaShaderTexture {
  GLint location;
  GLuint texture;
};

#define GAMMA_SHADER_TEXTURE_MAX 5

struct GammaShader {
  GLuint program;
  GLsizei texture_count;
  struct GammaShaderTexture texture[GAMMA_SHADER_TEXTURE_MAX];
};

// raw functions

#define GAMMA_INFO_LOG_MAX 1024

static GLuint gammaShaderRawCompile(const char *source, GLenum type) {
  GLuint shader = glCreateShader(type);

  if (shader == 0) {
    return 0;
  }

  glShaderSource(shader, 1, &source, NULL);
  glCompileShader(shader);

  GLint status = GL_FALSE;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

  if (status != GL_TRUE) {
    char info_log[GAMMA_INFO_LOG_MAX];
    glGetShaderInfoLog(shader, GAMMA_INFO_LOG_MAX, NULL, info_log);
    fprintf(stderr, "%s\n", info_log);
    return 0;
  }

  return shader;
}

static GLuint gammaShaderRawCompileProgram(const char *vertex_source, const char *fragment_source) {
  GLuint program = glCreateProgram();

  if (vertex_source != NULL) {
    GLuint shader = gammaShaderRawCompile(vertex_source, GL_VERTEX_SHADER);
    glAttachShader(program, shader);
    glDeleteShader(shader); // the shader is still here because it is attached to the program
  }

  if (fragment_source != NULL) {
    GLuint shader = gammaShaderRawCompile(fragment_source, GL_FRAGMENT_SHADER);
    glAttachShader(program, shader);
    glDeleteShader(shader); // the shader is still here because it is attached to the program
  }

  glLinkProgram(program);

  GLint status = GL_FALSE;
  glGetProgramiv(program, GL_LINK_STATUS, &status);

  if (status != GL_TRUE) {
    char info_log[GAMMA_INFO_LOG_MAX];
    glGetProgramInfoLog(program, GAMMA_INFO_LOG_MAX, NULL, info_log);
    fprintf(stderr, "%s\n", info_log);
    return 0;
  }

  return program;
}

// class


// methods


/*
 * Camera
 */

static const struct GammaVec2F gammaVec2FZero = {{ 0.0f, 0.0f }};
static const struct GammaVec2F gammaVec2FOne = {{ 1.0f, 1.0f }};
static const struct GammaRectF gammaDefaultViewport = { {{ 0.0f, 0.0f }}, {{ 1.0f, 1.0f }} };

enum GammaCameraType {
  GAMMA_CAMERA_EXTEND,
  GAMMA_CAMERA_FILL,
  GAMMA_CAMERA_FIT,
  GAMMA_CAMERA_LOCKED,
  GAMMA_CAMERA_SCREEN,
  GAMMA_CAMERA_STRETCH,
};

struct GammaCamera {
  enum GammaCameraType type;
  struct GammaVec2F center;
  struct GammaVec2F expected_size;
  struct GammaVec2F computed_size;
  float rotation;
  struct GammaRectF expected_viewport;
  struct GammaRectF computed_viewport;
};

// raw functions

static void gammaCameraRawCreate(struct GammaCamera *camera, enum GammaCameraType type, struct GammaVec2F center, struct GammaVec2F size) {
  camera->type = type;
  camera->center = center;
  camera->expected_size = size;
  camera->computed_size = size;
  camera->rotation = 0.0f;
  camera->expected_viewport = gammaDefaultViewport;
  camera->computed_viewport = gammaDefaultViewport;
}

static void gammaCameraRawUpdate(struct GammaCamera *camera, struct GammaVec2I framebuffer_size) {
  camera->computed_size = camera->expected_size;
  camera->computed_viewport = camera->expected_viewport;

  switch (camera->type) {
    case GAMMA_CAMERA_EXTEND: {
      float world_ratio = camera->expected_size.v[0] / camera->expected_size.v[1];

      struct GammaVec2F viewport_size;
      viewport_size.v[0] = framebuffer_size.v[0] * camera->expected_viewport.size.v[0];
      viewport_size.v[1] = framebuffer_size.v[1] * camera->expected_viewport.size.v[1];
      float framebuffer_ratio = viewport_size.v[0] / viewport_size.v[1];

      struct GammaVec2F size = camera->expected_size;

      if (framebuffer_ratio < world_ratio) {
        float ratio = framebuffer_ratio / world_ratio;
        size.v[1] /= ratio;
      } else {
        float ratio = world_ratio / framebuffer_ratio;
        size.v[0] /= ratio;
      }

      camera->computed_size = size;
      break;
    }
    case GAMMA_CAMERA_FILL: {
      float world_ratio = camera->expected_size.v[0] / camera->expected_size.v[1];

      struct GammaVec2F viewport_size;
      viewport_size.v[0] = framebuffer_size.v[0] * camera->expected_viewport.size.v[0];
      viewport_size.v[1] = framebuffer_size.v[1] * camera->expected_viewport.size.v[1];
      float framebuffer_ratio = viewport_size.v[0] / viewport_size.v[1];

      struct GammaVec2F size = camera->expected_size;

      if (framebuffer_ratio < world_ratio) {
        float ratio = framebuffer_ratio / world_ratio;
        size.v[0] *= ratio;
      } else {
        float ratio = world_ratio / framebuffer_ratio;
        size.v[1] *= ratio;
      }

      camera->computed_size = size;
      break;
    }
    case GAMMA_CAMERA_FIT: {
      float world_ratio = camera->expected_size.v[0] / camera->expected_size.v[1];

      struct GammaVec2F viewport_size;
      viewport_size.v[0] = framebuffer_size.v[0] * camera->expected_viewport.size.v[0];
      viewport_size.v[1] = framebuffer_size.v[1] * camera->expected_viewport.size.v[1];
      float framebuffer_ratio = viewport_size.v[0] / viewport_size.v[1];

      struct GammaRectF viewport;

      if (framebuffer_ratio < world_ratio) {
        float ratio = framebuffer_ratio / world_ratio;

        viewport.position.v[0] = 0.0f;
        viewport.size.v[0] = 1.0f;

        viewport.position.v[1] = (1.0f - ratio) / 2.0f;
        viewport.size.v[1] = ratio;
      } else {
        float ratio = world_ratio / framebuffer_ratio;

        viewport.position.v[1] = 0.0f;
        viewport.size.v[1] = 1.0f;

        viewport.position.v[0] = (1.0f - ratio) / 2.0f;
        viewport.size.v[0] = ratio;
      }

      camera->computed_viewport.position.v[0] = viewport.position.v[0] * camera->expected_viewport.size.v[0] + camera->expected_viewport.position.v[0];
      camera->computed_viewport.position.v[1] = viewport.position.v[1] * camera->expected_viewport.size.v[1] + camera->expected_viewport.position.v[1];
      camera->computed_viewport.size.v[0] = viewport.size.v[0] * camera->expected_viewport.size.v[0];
      camera->computed_viewport.size.v[1] = viewport.size.v[1] * camera->expected_viewport.size.v[1];
      break;
    }
    case GAMMA_CAMERA_LOCKED: {
      struct GammaVec2F size = camera->expected_size;

      struct GammaVec2F viewport_size;
      viewport_size.v[0] = framebuffer_size.v[0] * camera->expected_viewport.size.v[0];
      viewport_size.v[1] = framebuffer_size.v[1] * camera->expected_viewport.size.v[1];

      struct GammaRectF viewport;

      if (size.v[0] > viewport_size.v[0]) {
        viewport.position.v[0] = 0.0f;
        viewport.size.v[0] = 1.0f;
        size.v[0] = viewport_size.v[0];
      } else {
        viewport.size.v[0] = size.v[0] / viewport_size.v[0];
        viewport.position.v[0] = (1.0f - size.v[0]) / 2.0f;
      }

      if (size.v[1] > viewport_size.v[1]) {
        viewport.position.v[1] = 0.0f;
        viewport.size.v[1] = 1.0f;
        size.v[1] = viewport_size.v[1];
      } else {
        viewport.size.v[1] = size.v[1] / viewport_size.v[1];
        viewport.position.v[1] = (1.0f - size.v[1]) / 2.0f;
      }

      camera->computed_size = size;
      camera->computed_viewport = viewport;
      break;
    }
    case GAMMA_CAMERA_SCREEN: {
      struct GammaVec2F viewport_size;
      viewport_size.v[0] = framebuffer_size.v[0] * camera->expected_viewport.size.v[0];
      viewport_size.v[1] = framebuffer_size.v[1] * camera->expected_viewport.size.v[1];

      camera->computed_size = viewport_size;
      camera->center.v[0] = viewport_size.v[0] / 2.0f;
      camera->center.v[1] = viewport_size.v[1] / 2.0f;
      break;
    }
    case GAMMA_CAMERA_STRETCH:
      /* nothing to do */
      break;
  }
}

static void gammaCameraRawComputeViewMatrix(const struct GammaCamera *camera, struct GammaMat3F *mat) {
  float sx = 2.0f / camera->computed_size.v[0];
  float sy = - 2.0f / camera->computed_size.v[1];
  float cos_v = cosf(camera->rotation);
  float sin_v = sinf(camera->rotation);
  float tx = camera->center.v[0];
  float ty = camera->center.v[1];

  mat->m[0][0] =  sx * cos_v; mat->m[1][0] = sx * sin_v; mat->m[2][0] = sx * (- tx * cos_v - ty * sin_v);
  mat->m[0][1] = -sy * sin_v; mat->m[1][1] = sy * cos_v; mat->m[2][1] = sy * (  tx * sin_v - ty * cos_v);
  mat->m[0][2] = 0.0f;        mat->m[1][2] = 0.0f;       mat->m[2][2] = 1.0f;
}

static void gammaCameraRawComputeViewport(const struct GammaCamera *camera, struct GammaVec2I framebuffer_size, struct GammaRectI *viewport) {
  viewport->position.v[0] = (int) (framebuffer_size.v[0] * camera->computed_viewport.position.v[0] + 0.5f);
  viewport->position.v[1] = (int) (framebuffer_size.v[1] * camera->computed_viewport.position.v[1] + 0.5f);
  viewport->size.v[0] = (int) (framebuffer_size.v[0] * camera->computed_viewport.size.v[0] + 0.5f);
  viewport->size.v[1] = (int) (framebuffer_size.v[1] * camera->computed_viewport.size.v[1] + 0.5f);
}

// class

static ptrdiff_t gammaCameraAllocate(AgateVM *vm, const char *unit_name, const char *class_name) {
  return sizeof(struct GammaCamera);
}

static uint64_t gammaCameraTag(AgateVM *vm, const char *unit_name, const char *class_name) {
  return GAMMA_CAMERA_TAG;
}

// methods

#define GAMMA_CAMERA_NEW(name, type)                          \
static void gammaCameraNew ## name(AgateVM *vm) {             \
  assert(gammaCheckForeign(vm, 0, GAMMA_CAMERA_TAG));         \
  struct GammaCamera *camera = agateSlotGetForeign(vm, 0);    \
  struct GammaVec2F center;                                   \
  if (!gammaCheckVec2F(vm, 1, &center)) {                     \
    gammaError(vm, "Vec2F parameter expected for `center`."); \
    return;                                                   \
  }                                                           \
  struct GammaVec2F size;                                     \
  if (!gammaCheckVec2F(vm, 2, &size)) {                       \
    gammaError(vm, "Vec2F parameter expected for `size`.");   \
    return;                                                   \
  }                                                           \
  gammaCameraRawCreate(camera, type, center, size);           \
}

GAMMA_CAMERA_NEW(Extend, GAMMA_CAMERA_EXTEND)
GAMMA_CAMERA_NEW(Fill, GAMMA_CAMERA_FILL)
GAMMA_CAMERA_NEW(Fit, GAMMA_CAMERA_FIT)
GAMMA_CAMERA_NEW(Locked, GAMMA_CAMERA_LOCKED)
GAMMA_CAMERA_NEW(Screen, GAMMA_CAMERA_SCREEN)
GAMMA_CAMERA_NEW(Stretch, GAMMA_CAMERA_STRETCH)

static void gammaCameraGetSize(AgateVM *vm) {
  assert(gammaCheckForeign(vm, 0, GAMMA_CAMERA_TAG));
  struct GammaCamera *camera = agateSlotGetForeign(vm, 0);

  struct GammaVec2F *result = gammaForeignAllocate(vm, AGATE_RETURN_SLOT, "Vec2F");
  *result = camera->expected_size;
}

static void gammaCameraSetSize(AgateVM *vm) {
  assert(gammaCheckForeign(vm, 0, GAMMA_CAMERA_TAG));
  struct GammaCamera *camera = agateSlotGetForeign(vm, 0);

  if (!gammaCheckVec2F(vm, 1, &camera->expected_size)) {
    gammaError(vm, "Vec2F parameter expected for `value`.");
    return;
  }
}

static void gammaCameraGetCenter(AgateVM *vm) {
  assert(gammaCheckForeign(vm, 0, GAMMA_CAMERA_TAG));
  struct GammaCamera *camera = agateSlotGetForeign(vm, 0);

  struct GammaVec2F *result = gammaForeignAllocate(vm, AGATE_RETURN_SLOT, "Vec2F");
  *result = camera->center;
}

static void gammaCameraSetCenter(AgateVM *vm) {
  assert(gammaCheckForeign(vm, 0, GAMMA_CAMERA_TAG));
  struct GammaCamera *camera = agateSlotGetForeign(vm, 0);

  if (!gammaCheckVec2F(vm, 1, &camera->center)) {
    gammaError(vm, "Vec2F parameter expected for `value`.");
    return;
  }
}

static void gammaCameraMove1(AgateVM *vm) {
  assert(gammaCheckForeign(vm, 0, GAMMA_CAMERA_TAG));
  struct GammaCamera *camera = agateSlotGetForeign(vm, 0);

  struct GammaVec2F offset;

  if (!gammaCheckVec2F(vm, 1, &offset)) {
    gammaError(vm, "Vec2F parameter expected for `offset`.");
    return;
  }

  camera->center.v[0] += offset.v[0];
  camera->center.v[1] += offset.v[1];
}

static void gammaCameraMove2(AgateVM *vm) {
  assert(gammaCheckForeign(vm, 0, GAMMA_CAMERA_TAG));
  struct GammaCamera *camera = agateSlotGetForeign(vm, 0);

  float x;

  if (!gammaCheckFloat(vm, 1, &x)) {
    gammaError(vm, "Float parameter expected for `x`.");
    return;
  }

  float y;

  if (!gammaCheckFloat(vm, 2, &y)) {
    gammaError(vm, "Float parameter expected for `y`.");
    return;
  }

  camera->center.v[0] += x;
  camera->center.v[1] += y;
}

static void gammaCameraGetRotation(AgateVM *vm) {
  assert(gammaCheckForeign(vm, 0, GAMMA_CAMERA_TAG));
  struct GammaCamera *camera = agateSlotGetForeign(vm, 0);
  agateSlotSetFloat(vm, AGATE_RETURN_SLOT, camera->rotation);
}

static void gammaCameraSetRotation(AgateVM *vm) {
  assert(gammaCheckForeign(vm, 0, GAMMA_CAMERA_TAG));
  struct GammaCamera *camera = agateSlotGetForeign(vm, 0);

  if (!gammaCheckFloat(vm, 1, &camera->rotation)) {
    gammaError(vm, "Float parameter expected for `value`.");
    return;
  }
}

static void gammaCameraRotate(AgateVM *vm) {
  assert(gammaCheckForeign(vm, 0, GAMMA_CAMERA_TAG));
  struct GammaCamera *camera = agateSlotGetForeign(vm, 0);

  float angle;

  if (!gammaCheckFloat(vm, 1, &angle)) {
    gammaError(vm, "Float parameter expected for `angle`.");
    return;
  }

  camera->rotation += angle;
}

static void gammaCameraZoom1(AgateVM *vm) {
  assert(gammaCheckForeign(vm, 0, GAMMA_CAMERA_TAG));
  struct GammaCamera *camera = agateSlotGetForeign(vm, 0);
  float factor = 1.0;

  if (!gammaCheckFloat(vm, 1, &factor)) {
    gammaError(vm, "Float parameter expected for `factor`.");
    return;
  }

  camera->expected_size.v[0] *= factor;
  camera->expected_size.v[1] *= factor;
}

static void gammaCameraZoom2(AgateVM *vm) {
  assert(gammaCheckForeign(vm, 0, GAMMA_CAMERA_TAG));
  struct GammaCamera *camera = agateSlotGetForeign(vm, 0);
  float factor = 1.0;

  if (!gammaCheckFloat(vm, 1, &factor)) {
    gammaError(vm, "Float parameter expected for `factor`.");
    return;
  }

  struct GammaVec2F fixed;

  if (!gammaCheckVec2F(vm, 1, &fixed)) {
    gammaError(vm, "Vec2F parameter expected for `fixed`.");
    return;
  }

  camera->center.v[0] += (fixed.v[0] - camera->center.v[0]) * (1.0f - factor);
  camera->center.v[1] += (fixed.v[1] - camera->center.v[1]) * (1.0f - factor);

  camera->expected_size.v[0] *= factor;
  camera->expected_size.v[1] *= factor;
}

static void gammaCameraGetViewport(AgateVM *vm) {
  assert(gammaCheckForeign(vm, 0, GAMMA_CAMERA_TAG));
  struct GammaCamera *camera = agateSlotGetForeign(vm, 0);

  struct GammaRectF *result = gammaForeignAllocate(vm, AGATE_RETURN_SLOT, "RectF");
  *result = camera->expected_viewport;
}

static void gammaCameraSetViewport(AgateVM *vm) {
  assert(gammaCheckForeign(vm, 0, GAMMA_CAMERA_TAG));
  struct GammaCamera *camera = agateSlotGetForeign(vm, 0);

  if (!gammaCheckRectF(vm, 1, &camera->expected_viewport)) {
    gammaError(vm, "RectF parameter expected for `value`.");
    return;
  }
}

/*
 * Transform
 */

struct GammaTransform {
  struct GammaVec2F origin;
  struct GammaVec2F position;
  float rotation;
  struct GammaVec2F scale;
};

// raw

static void gammaTransformComputeMatrix(const struct GammaTransform *transform, struct GammaRectF bounds, struct GammaMat3F *mat) {
  float ox = transform->origin.v[0] * bounds.size.v[0] + bounds.position.v[0];
  float oy = transform->origin.v[1] * bounds.size.v[1] + bounds.position.v[0];
  float px = transform->position.v[0];
  float py = transform->position.v[1];
  float cos_v = cosf(transform->rotation);
  float sin_v = sinf(transform->rotation);
  float sx = transform->scale.v[0];
  float sy = transform->scale.v[1];

  mat->m[0][0] = sx * cos_v; mat->m[1][0] = - sy * sin_v; mat->m[2][0] = - ox * mat->m[0][0] - oy * mat->m[1][0] + px;
  mat->m[0][1] = sx * sin_v; mat->m[1][1] =   sy * cos_v; mat->m[2][1] = - ox * mat->m[0][1] - oy * mat->m[1][1] + py;
  mat->m[0][2] = 0.0f;       mat->m[1][2] = 0.0f;         mat->m[2][2] = 1.0f;
}

// class

static ptrdiff_t gammaTransformAllocate(AgateVM *vm, const char *unit_name, const char *class_name) {
  return sizeof(struct GammaTransform);
}

static uint64_t gammaTransformTag(AgateVM *vm, const char *unit_name, const char *class_name) {
  return GAMMA_TRANSFORM_TAG;
}

// methods

static void gammaTransformNew0(AgateVM *vm) {
  assert(gammaCheckForeign(vm, 0, GAMMA_TRANSFORM_TAG));
  struct GammaTransform *transform = agateSlotGetForeign(vm, 0);

  transform->origin = gammaVec2FZero;
  transform->position = gammaVec2FZero;
  transform->rotation = 0.0f;
  transform->scale = gammaVec2FOne;
}

static void gammaTransformNew1(AgateVM *vm) {
  assert(gammaCheckForeign(vm, 0, GAMMA_TRANSFORM_TAG));
  struct GammaTransform *transform = agateSlotGetForeign(vm, 0);

  struct GammaVec2F position;

  if (!gammaCheckVec2F(vm, 1, &position)) {
    gammaError(vm, "Vec2F parameter expected for `position`.");
    return;
  }

  transform->origin = gammaVec2FZero;
  transform->position = position;
  transform->rotation = 0.0f;
  transform->scale = gammaVec2FOne;
}

static void gammaTransformNew2(AgateVM *vm) {
  assert(gammaCheckForeign(vm, 0, GAMMA_TRANSFORM_TAG));
  struct GammaTransform *transform = agateSlotGetForeign(vm, 0);

  struct GammaVec2F position;

  if (!gammaCheckVec2F(vm, 1, &position)) {
    gammaError(vm, "Vec2F parameter expected for `position`.");
    return;
  }

  struct GammaVec2F origin;

  if (!gammaCheckVec2F(vm, 2, &origin)) {
    gammaError(vm, "Vec2F parameter expected for `origin`.");
    return;
  }

  transform->origin = origin;
  transform->position = position;
  transform->rotation = 0.0f;
  transform->scale = gammaVec2FOne;
}

static void gammaTransformNew3(AgateVM *vm) {
  assert(gammaCheckForeign(vm, 0, GAMMA_TRANSFORM_TAG));
  struct GammaTransform *transform = agateSlotGetForeign(vm, 0);

  struct GammaVec2F position;

  if (!gammaCheckVec2F(vm, 1, &position)) {
    gammaError(vm, "Vec2F parameter expected for `position`.");
    return;
  }

  struct GammaVec2F origin;

  if (!gammaCheckVec2F(vm, 2, &origin)) {
    gammaError(vm, "Vec2F parameter expected for `origin`.");
    return;
  }

  float rotation;

  if (!gammaCheckFloat(vm, 3, &rotation)) {
    gammaError(vm, "Float parameter expected for `angle`.");
    return;
  }

  transform->origin = origin;
  transform->position = position;
  transform->rotation = rotation;
  transform->scale = gammaVec2FOne;
}

static void gammaTransformGetOrigin(AgateVM *vm) {
  assert(gammaCheckForeign(vm, 0, GAMMA_TRANSFORM_TAG));
  struct GammaTransform *transform = agateSlotGetForeign(vm, 0);

  struct GammaVec2F *result = gammaForeignAllocate(vm, AGATE_RETURN_SLOT, "Vec2F");
  *result = transform->origin;
}

static void gammaTransformSetOrigin(AgateVM *vm) {
  assert(gammaCheckForeign(vm, 0, GAMMA_TRANSFORM_TAG));
  struct GammaTransform *transform = agateSlotGetForeign(vm, 0);

  if (!gammaCheckVec2F(vm, 1, &transform->origin)) {
    gammaError(vm, "Vec2F parameter expected for `value`.");
    return;
  }

  agateSlotCopy(vm, AGATE_RETURN_SLOT, 1);
}

static void gammaTransformGetPosition(AgateVM *vm) {
  assert(gammaCheckForeign(vm, 0, GAMMA_TRANSFORM_TAG));
  struct GammaTransform *transform = agateSlotGetForeign(vm, 0);

  struct GammaVec2F *result = gammaForeignAllocate(vm, AGATE_RETURN_SLOT, "Vec2F");
  *result = transform->position;
}

static void gammaTransformSetPosition(AgateVM *vm) {
  assert(gammaCheckForeign(vm, 0, GAMMA_TRANSFORM_TAG));
  struct GammaTransform *transform = agateSlotGetForeign(vm, 0);

  if (!gammaCheckVec2F(vm, 1, &transform->position)) {
    gammaError(vm, "Vec2F parameter expected for `value`.");
    return;
  }

  agateSlotCopy(vm, AGATE_RETURN_SLOT, 1);
}

static void gammaTransformMove(AgateVM *vm) {
  assert(gammaCheckForeign(vm, 0, GAMMA_TRANSFORM_TAG));
  struct GammaTransform *transform = agateSlotGetForeign(vm, 0);

  struct GammaVec2F offset;

  if (!gammaCheckVec2F(vm, 1, &offset)) {
    gammaError(vm, "Vec2F parameter expected for `offset`.");
    return;
  }

  transform->position.v[0] += offset.v[0];
  transform->position.v[1] += offset.v[1];
}

static void gammaTransformGetRotation(AgateVM *vm) {
  assert(gammaCheckForeign(vm, 0, GAMMA_TRANSFORM_TAG));
  struct GammaTransform *transform = agateSlotGetForeign(vm, 0);
  agateSlotSetFloat(vm, AGATE_RETURN_SLOT, transform->rotation);
}

static void gammaTransformSetRotation(AgateVM *vm) {
  assert(gammaCheckForeign(vm, 0, GAMMA_TRANSFORM_TAG));
  struct GammaTransform *transform = agateSlotGetForeign(vm, 0);

  if (!gammaCheckFloat(vm, 1, &transform->rotation)) {
    gammaError(vm, "Float parameter expected for `value`.");
    return;
  }

  agateSlotCopy(vm, AGATE_RETURN_SLOT, 1);
}

static void gammaTransformRotate(AgateVM *vm) {
  assert(gammaCheckForeign(vm, 0, GAMMA_TRANSFORM_TAG));
  struct GammaTransform *transform = agateSlotGetForeign(vm, 0);

  float angle;

  if (!gammaCheckFloat(vm, 1, &angle)) {
    gammaError(vm, "Float parameter expected for `angle`.");
    return;
  }

  transform->rotation += angle;
}

static void gammaTransformGetScale(AgateVM *vm) {
  assert(gammaCheckForeign(vm, 0, GAMMA_TRANSFORM_TAG));
  struct GammaTransform *transform = agateSlotGetForeign(vm, 0);

  struct GammaVec2F *result = gammaForeignAllocate(vm, AGATE_RETURN_SLOT, "Vec2F");
  *result = transform->scale;
}

static void gammaTransformSetScale(AgateVM *vm) {
  assert(gammaCheckForeign(vm, 0, GAMMA_TRANSFORM_TAG));
  struct GammaTransform *transform = agateSlotGetForeign(vm, 0);

  if (!gammaCheckVec2F(vm, 1, &transform->scale)) {
    gammaError(vm, "Vec2F parameter expected for `value`.");
    return;
  }

  agateSlotCopy(vm, AGATE_RETURN_SLOT, 1);
}

static void gammaTransformScale(AgateVM *vm) {
  assert(gammaCheckForeign(vm, 0, GAMMA_TRANSFORM_TAG));
  struct GammaTransform *transform = agateSlotGetForeign(vm, 0);

  struct GammaVec2F factor;

  if (!gammaCheckVec2F(vm, 1, &factor)) {
    gammaError(vm, "Vec2F parameter expected for `factor`.");
    return;
  }

  transform->scale.v[0] *= factor.v[0];
  transform->scale.v[1] *= factor.v[1];
}

/*
 * Renderer
 */

struct GammaVertex {
  struct GammaVec2F position;
  struct GammaColor color;
  struct GammaVec2F texcoords;
};

struct GammaRendererData {
  GLenum mode;
  GLsizei count;
  GLuint vertex_buffer;
  GLuint element_buffer;
  enum GammaTextureFormat format;
  GLuint texture0;
  GLuint texture1;
  GLuint shader;
  struct GammaMat3F transform;
};

struct GammaRenderer {
  SDL_GLContext context;
  GLuint vao;
  GLuint default_shader;
  GLuint default_alpha_shader;
  GLuint default_texture;

  struct GammaVec2I framebuffer_size;
  struct GammaCamera camera;
};

// raw functions

static void gammaRendererRawCreate(struct GammaRenderer *renderer, struct GammaWindow *window, AgateVM *vm) {
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

  renderer->context = SDL_GL_CreateContext(window->ptr);

  if (renderer->context == NULL) {
    gammaError(vm, "Unable to create a context: %s\n", SDL_GetError());
    return;
  }

  if (SDL_GL_MakeCurrent(window->ptr, renderer->context) != 0) {
    gammaError(vm, "Unable to make the context current: %s\n", SDL_GetError());
    return;
  }

  if (gladLoadGLLoader(SDL_GL_GetProcAddress) == 0) {
    gammaError(vm, "Unable to load OpenGL 3.3.\n");
    return;
  }

  glEnable(GL_BLEND);
  glEnable(GL_SCISSOR_TEST);
  glEnable(GL_COLOR_BUFFER_BIT);

  glGenVertexArrays(1, &renderer->vao);
  glBindVertexArray(renderer->vao);

  renderer->default_shader = gammaShaderRawCompileProgram(gammaDefaultVert, gammaDefaultFrag);
  renderer->default_alpha_shader = gammaShaderRawCompileProgram(gammaDefaultVert, gammaDefaultAlphaFrag);

  const uint8_t pixel[] = { 0xFF, 0xFF, 0xFF, 0xFF };

  GLuint texture = gammaTextureRawCreate(1, 1, GL_RGBA, 4, pixel);
  assert(texture != 0);

  glBindTexture(GL_TEXTURE_2D, texture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glBindTexture(GL_TEXTURE_2D, 0);

  renderer->default_texture = texture;

  SDL_GL_GetDrawableSize(window->ptr, &renderer->framebuffer_size.v[0], &renderer->framebuffer_size.v[1]);

  gammaCameraRawCreate(&renderer->camera, GAMMA_CAMERA_SCREEN, gammaVec2FZero, gammaVec2FOne);
  gammaCameraRawUpdate(&renderer->camera, renderer->framebuffer_size);
}

static void gammaRendererRawDestroy(struct GammaRenderer *renderer) {
  if (renderer->context == NULL) {
    return;
  }

  glBindTexture(GL_TEXTURE_2D, 0);

  if (renderer->default_texture != 0) {
    glDeleteTextures(1, &renderer->default_texture);
    renderer->default_texture = 0;
  }

  glUseProgram(0);

  if (renderer->default_alpha_shader != 0) {
    glDeleteProgram(renderer->default_alpha_shader);
    renderer->default_alpha_shader = 0;
  }

  if (renderer->default_shader != 0) {
    glDeleteProgram(renderer->default_shader);
    renderer->default_shader = 0;
  }

  glBindVertexArray(0);
  glDeleteVertexArrays(1, &renderer->vao);
  renderer->vao = 0;

  SDL_GL_DeleteContext(renderer->context);
  renderer->context = NULL;
}

static void gammaRendererRawDraw(const struct GammaRenderer *renderer, const struct GammaRendererData *submitted_data) {
  if (submitted_data->vertex_buffer == 0) {
    return;
  }

  struct GammaRendererData data = *submitted_data;

  // textures

  if (data.texture0 == 0) {
    data.texture0 = renderer->default_texture;
  }

  if (data.texture1 == 0) {
    data.texture1 = renderer->default_texture;
  }

  // shader

  if (data.shader == 0) {
    if (data.format == GAMMA_TEXTURE_ALPHA) {
      data.shader = renderer->default_alpha_shader;
    } else {
      data.shader = renderer->default_shader;
    }
  }

  glUseProgram(data.shader);
  GLint index = 0;

  GLint texture0_location = glGetUniformLocation(data.shader, "texture0");

  if (texture0_location != -1) {
    glActiveTexture(GL_TEXTURE0 + index);
    glUniform1i(texture0_location, index++);
    glBindTexture(GL_TEXTURE_2D, data.texture0);
  }

  GLint texture1_location = glGetUniformLocation(data.shader, "texture1");

  if (texture1_location != -1) {
    glActiveTexture(GL_TEXTURE0 + index);
    glUniform1i(texture1_location, index++);
    glBindTexture(GL_TEXTURE_2D, data.texture1);
  }

  // transform

  struct GammaMat3F view;
  gammaCameraRawComputeViewMatrix(&renderer->camera, &view);

  struct GammaMat3F transform;
  gammaMat3FRawMul(&transform, &view, &data.transform);

  GLint transform_location = glGetUniformLocation(data.shader, "transform");

  if (transform_location != -1) {
    glUniformMatrix3fv(transform_location, 1, GL_FALSE, &transform.m[0][0]);
  }

  // blend

  glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
  glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

  // buffers

  glBindBuffer(GL_ARRAY_BUFFER, data.vertex_buffer);

  if (data.element_buffer != 0) {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, data.element_buffer);
  }

  // inputs

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(struct GammaVertex), (void *) offsetof(struct GammaVertex, position));

  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(struct GammaVertex), (void *) offsetof(struct GammaVertex, color));

  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(struct GammaVertex), (void *) offsetof(struct GammaVertex, texcoords));

  // draw

  if (data.element_buffer == 0) {
    glDrawArrays(data.mode, 0, data.count);
  } else {
    glDrawElements(data.mode, data.count, GL_UNSIGNED_SHORT, NULL);
  }

  // cleanup

  glDisableVertexAttribArray(2);
  glDisableVertexAttribArray(1);
  glDisableVertexAttribArray(0);

  glBindBuffer(GL_ARRAY_BUFFER, 0);

  if (data.element_buffer != 0) {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  }
}

static struct GammaVec2I gammaRendererRawWorldToDevice(const struct GammaRenderer *renderer, const struct GammaCamera *camera, struct GammaVec2F position) {
  struct GammaRectI viewport;
  gammaCameraRawComputeViewport(camera, renderer->framebuffer_size, &viewport);

  /* apply view transform
   * i.e. compute normalized device coordinates from world coordinates
   */

  struct GammaMat3F view;
  gammaCameraRawComputeViewMatrix(camera, &view);

  struct GammaVec2F normalized;
  gammaMat3FRawTransformPoint(&normalized, &view, &position);

  /* simulate projection transform
   * i.e. compute screen coordinates from normalized device coordinates
   *
   *  1 +---------+     0 +---------+
   *    |         |       |         |
   *    |         | ===>  |         |
   *    |         |       |         |
   * -1 +---------+     h +---------+
   *   -1         1       0         w
   */

  struct GammaVec2I result;
  result.v[0] = (int) (1 + normalized.v[0] / 2 * viewport.size.v[0] + viewport.position.v[0]);
  result.v[1] = (int) (1 - normalized.v[1] / 2 * viewport.size.v[1] + viewport.position.v[1]);
  return result;
}

static struct GammaVec2F gammaRendererRawDeviceToWorld(const struct GammaRenderer *renderer, const struct GammaCamera *camera, struct GammaVec2I coordinates) {
  struct GammaRectI viewport;
  gammaCameraRawComputeViewport(camera, renderer->framebuffer_size, &viewport);

  /* simulate inverse projection transform
   * i.e. compute normalized device coordinates from screen coordinates
   *
   * 0 +---------+      1 +---------+
   *   |         |        |         |
   *   |         | ===>   |         |
   *   |         |        |         |
   * h +---------+     -1 +---------+
   *   0         w       -1         1
   */

  struct GammaVec2F normalized;
  normalized.v[0] = 2.0f * (coordinates.v[0] - viewport.position.v[0]) / viewport.size.v[0] - 1.0f;
  normalized.v[1] = 1.0f - 2.0f * (coordinates.v[1] - viewport.position.v[1]) / viewport.size.v[1];

  /* apply inverse view transform
   * i.e. compute world coordinates from normalized device coordinates
   */

  struct GammaMat3F view;
  gammaCameraRawComputeViewMatrix(camera, &view);

  struct GammaMat3F inverse_view;
  bool ok = gammaMat3FRawInverse(&inverse_view, &view);
  assert(ok);

  struct GammaVec2F result;
  gammaMat3FRawTransformPoint(&result, &inverse_view, &normalized);
  return result;
}

// class

static ptrdiff_t gammaRendererAllocate(AgateVM *vm, const char *unit_name, const char *class_name) {
  return sizeof(struct GammaRenderer);
}

static uint64_t gammaRendererTag(AgateVM *vm, const char *unit_name, const char *class_name) {
  return GAMMA_RENDERER_TAG;
}

static void gammaRendererDestroy(AgateVM *vm, const char *unit_name, const char *class_name, void *data) {
  struct GammaRenderer *renderer = data;
  gammaRendererRawDestroy(renderer);
}

// methods

static void gammaRendererNew(AgateVM *vm) {
  assert(gammaCheckForeign(vm, 0, GAMMA_RENDERER_TAG));
  struct GammaRenderer *renderer = agateSlotGetForeign(vm, 0);

  if (!gammaCheckForeign(vm, 1, GAMMA_WINDOW_TAG)) {
    gammaError(vm, "Window parameter expected for `window`.");
    return;
  }

  struct GammaWindow *window = agateSlotGetForeign(vm, 1);

  gammaRendererRawCreate(renderer, window, vm);
}

static void gammaRendererClear0(AgateVM *vm) {
  assert(gammaCheckForeign(vm, 0, GAMMA_RENDERER_TAG));
  struct GammaRenderer *renderer = agateSlotGetForeign(vm, 0);

  glScissor(0, 0, renderer->framebuffer_size.v[0], renderer->framebuffer_size.v[1]);
  glClear(GL_COLOR_BUFFER_BIT);
}

static void gammaRendererClear1(AgateVM *vm) {
  struct GammaColor color;

  if (!gammaCheckColor(vm, 1, &color)) {
    gammaError(vm, "Color parameter expected for `color`.");
    return;
  }

  glClearColor(color.r, color.g, color.b, color.a);
  glClear(GL_COLOR_BUFFER_BIT);
}

static void gammaRendererDisplay(AgateVM *vm) {
  SDL_GL_SwapWindow(SDL_GL_GetCurrentWindow());
}

static void gammaRendererSetCamera(AgateVM *vm) {
  assert(gammaCheckForeign(vm, 0, GAMMA_RENDERER_TAG));
  struct GammaRenderer *renderer = agateSlotGetForeign(vm, 0);

  if (!gammaCheckForeign(vm, 1, GAMMA_CAMERA_TAG)) {
    gammaError(vm, "Camera parameter expected for `value`.");
    return;
  }

  const struct GammaCamera *camera = agateSlotGetForeign(vm, 1);

  renderer->camera = *camera;

  SDL_GL_GetDrawableSize(SDL_GL_GetCurrentWindow(), &renderer->framebuffer_size.v[0], &renderer->framebuffer_size.v[1]);
  gammaCameraRawUpdate(&renderer->camera, renderer->framebuffer_size);

  struct GammaRectI viewport;
  gammaCameraRawComputeViewport(&renderer->camera, renderer->framebuffer_size, &viewport);
  viewport.position.v[1] = renderer->framebuffer_size.v[1] - (viewport.position.v[1] + viewport.size.v[1]); // invert y

  // set the viewport everytime a new camera is defined
  glViewport(viewport.position.v[0], viewport.position.v[1], viewport.size.v[0], viewport.size.v[1]);

  // the viewport does not scissor
  glScissor(viewport.position.v[0], viewport.position.v[1], viewport.size.v[0], viewport.size.v[1]);
}

static void gammaRendererWorldToDevice1(AgateVM *vm) {
  assert(gammaCheckForeign(vm, 0, GAMMA_RENDERER_TAG));
  struct GammaRenderer *renderer = agateSlotGetForeign(vm, 0);

  struct GammaVec2F position;

  if (!gammaCheckVec2F(vm, 1, &position)) {
    gammaError(vm, "Vec2F parameter expected for `position`.");
    return;
  }

  struct GammaVec2I *result = gammaForeignAllocate(vm, AGATE_RETURN_SLOT, "Vec2I");
  *result = gammaRendererRawWorldToDevice(renderer, &renderer->camera, position);
}

static void gammaRendererWorldToDevice2(AgateVM *vm) {
  assert(gammaCheckForeign(vm, 0, GAMMA_RENDERER_TAG));
  struct GammaRenderer *renderer = agateSlotGetForeign(vm, 0);

  struct GammaVec2F position;

  if (!gammaCheckVec2F(vm, 1, &position)) {
    gammaError(vm, "Vec2F parameter expected for `position`.");
    return;
  }

  if (!gammaCheckForeign(vm, 2, GAMMA_CAMERA_TAG)) {
    gammaError(vm, "Camera parameter expected for `camera`.");
    return;
  }

  const struct GammaCamera *camera = agateSlotGetForeign(vm, 2);

  struct GammaVec2I *result = gammaForeignAllocate(vm, AGATE_RETURN_SLOT, "Vec2I");
  *result = gammaRendererRawWorldToDevice(renderer, camera, position);
}

static void gammaRendererDeviceToWorld1(AgateVM *vm) {
  assert(gammaCheckForeign(vm, 0, GAMMA_RENDERER_TAG));
  struct GammaRenderer *renderer = agateSlotGetForeign(vm, 0);

  struct GammaVec2I coordinates;

  if (!gammaCheckVec2I(vm, 1, &coordinates)) {
    gammaError(vm, "Vec2I parameter expected for `coordinates`.");
    return;
  }

  struct GammaVec2F *result = gammaForeignAllocate(vm, AGATE_RETURN_SLOT, "Vec2F");
  *result = gammaRendererRawDeviceToWorld(renderer, &renderer->camera, coordinates);
}

static void gammaRendererDeviceToWorld2(AgateVM *vm) {
  assert(gammaCheckForeign(vm, 0, GAMMA_RENDERER_TAG));
  struct GammaRenderer *renderer = agateSlotGetForeign(vm, 0);

  struct GammaVec2I coordinates;

  if (!gammaCheckVec2I(vm, 1, &coordinates)) {
    gammaError(vm, "Vec2I parameter expected for `coordinates`.");
    return;
  }

  if (!gammaCheckForeign(vm, 2, GAMMA_CAMERA_TAG)) {
    gammaError(vm, "Camera parameter expected for `camera`.");
    return;
  }

  const struct GammaCamera *camera = agateSlotGetForeign(vm, 2);

  struct GammaVec2F *result = gammaForeignAllocate(vm, AGATE_RETURN_SLOT, "Vec2F");
  *result = gammaRendererRawDeviceToWorld(renderer, camera, coordinates);
}

static void gammaRendererDrawObject(AgateVM *vm) {
  assert(gammaCheckForeign(vm, 0, GAMMA_RENDERER_TAG));
  struct GammaRenderer *renderer = agateSlotGetForeign(vm, 0);

  if (agateSlotType(vm, 1) != AGATE_TYPE_FOREIGN) {
    gammaError(vm, "Graphical object parameter expected for `object`.");
    return;
  }

  if (!gammaCheckForeign(vm, 2, GAMMA_TRANSFORM_TAG)) {
    gammaError(vm, "Transform parameter expected for `transform`.");
    return;
  }

  struct GammaTransform *transform = agateSlotGetForeign(vm, 2);

  struct GammaRendererData data;

  switch (agateSlotGetForeignTag(vm, 1)) {



    default:
      gammaError(vm, "Graphical object parameter expected for `object`.");
      break;
  }

}

static void gammaRendererDrawRect2(AgateVM *vm) {
  assert(gammaCheckForeign(vm, 0, GAMMA_RENDERER_TAG));
  struct GammaRenderer *renderer = agateSlotGetForeign(vm, 0);

  struct GammaRectF rect;

  if (!gammaCheckRectF(vm, 1, &rect)) {
    gammaError(vm, "RectF parameter expected for `rect`.");
    return;
  }

  struct GammaColor color;

  if (!gammaCheckColor(vm, 2, &color)) {
    gammaError(vm, "Color parameter expected for `color`.");
    return;
  }

  struct GammaVertex vertices[] = {
    { {{ 0.0f,            0.0f            }}, color, gammaVec2FZero },
    { {{ 0.0f,            rect.size.v[1]  }}, color, gammaVec2FZero },
    { {{ rect.size.v[0],  0.0f            }}, color, gammaVec2FZero },
    { {{ rect.size.v[0],  rect.size.v[1]  }}, color, gammaVec2FZero },
  };

  struct GammaRendererData data;

  data.mode = GL_TRIANGLE_STRIP;
  data.count = 4;

  glGenBuffers(1, &data.vertex_buffer);
  glBindBuffer(GL_ARRAY_BUFFER, data.vertex_buffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  data.element_buffer = 0;
  data.format = GAMMA_TEXTURE_COLOR;
  data.texture0 = 0;
  data.texture1 = 0;
  data.shader = 0;

  gammaMat3FRawTranslation(&data.transform, rect.position);

  gammaRendererRawDraw(renderer, &data);

  glDeleteBuffers(1, &data.vertex_buffer);
}


static void gammaRendererIsVsynced(AgateVM *vm) {
  agateSlotSetBool(vm, AGATE_RETURN_SLOT, SDL_GL_GetSwapInterval() != 0);
}

static void gammaRendererSetVsynced(AgateVM *vm) {
  bool vsynced;

  if (!gammaCheckBool(vm, 1, &vsynced)) {
    gammaError(vm, "Bool parameter expected for `value`.");
    agateSlotSetNil(vm, AGATE_RETURN_SLOT);
    return;
  }

  if (SDL_GL_SetSwapInterval(vsynced ? 1 : 0) != 0) {
    if (vsynced) {
      gammaError(vm, "Unable to set vertical synchronization.");
    } else {
      gammaError(vm, "Unable to unset vertical synchronization.");
    }

    agateSlotSetNil(vm, AGATE_RETURN_SLOT);
    return;
  }

  agateSlotCopy(vm, AGATE_RETURN_SLOT, 1);
}


/*
 * unit configuration
 */

AgateForeignClassHandler gammaGfxClassHandler(AgateVM *vm, const char *unit_name, const char *class_name) {
  assert(gammaEquals(unit_name, "gamma/gfx"));
  AgateForeignClassHandler handler = { NULL, NULL, NULL };

  if (gammaEquals(class_name, "Camera")) {
    handler.allocate = gammaCameraAllocate;
    handler.tag = gammaCameraTag;
    return handler;
  }

  if (gammaEquals(class_name, "Renderer")) {
    handler.allocate = gammaRendererAllocate;
    handler.tag = gammaRendererTag;
    handler.destroy = gammaRendererDestroy;
    return handler;
  }

  if (gammaEquals(class_name, "Transform")) {
    handler.allocate = gammaTransformAllocate;
    handler.tag = gammaTransformTag;
    return handler;
  }

  return handler;
}

AgateForeignMethodFunc gammaGfxMethodHandler(AgateVM *vm, const char *unit_name, const char *class_name, AgateForeignMethodKind kind, const char *signature) {
  assert(gammaEquals(unit_name, "gamma/gfx"));

  if (gammaEquals(class_name, "Camera")) {
    if (kind == AGATE_FOREIGN_METHOD_INSTANCE) {
      if (gammaEquals(signature, "init new_extend(_,_)")) { return gammaCameraNewExtend; }
      if (gammaEquals(signature, "init new_fill(_,_)")) { return gammaCameraNewFill; }
      if (gammaEquals(signature, "init new_fit(_,_)")) { return gammaCameraNewFit; }
      if (gammaEquals(signature, "init new_locked(_,_)")) { return gammaCameraNewLocked; }
      if (gammaEquals(signature, "init new_screen(_,_)")) { return gammaCameraNewScreen; }
      if (gammaEquals(signature, "init new_stretch(_,_)")) { return gammaCameraNewStretch; }
      if (gammaEquals(signature, "size")) { return gammaCameraGetSize; }
      if (gammaEquals(signature, "size=(_)")) { return gammaCameraSetSize; }
      if (gammaEquals(signature, "center")) { return gammaCameraGetCenter; }
      if (gammaEquals(signature, "center=(_)")) { return gammaCameraSetCenter; }
      if (gammaEquals(signature, "move(_)")) { return gammaCameraMove1; }
      if (gammaEquals(signature, "move(_,_)")) { return gammaCameraMove2; }
      if (gammaEquals(signature, "rotate(_)")) { return gammaCameraRotate; }
      if (gammaEquals(signature, "rotation")) { return gammaCameraGetRotation; }
      if (gammaEquals(signature, "rotation=(_)")) { return gammaCameraSetRotation; }
      if (gammaEquals(signature, "rotate(_)")) { return gammaCameraRotate; }
      if (gammaEquals(signature, "zoom(_)")) { return gammaCameraZoom1; }
      if (gammaEquals(signature, "zoom(_,_)")) { return gammaCameraZoom2; }
      if (gammaEquals(signature, "viewport")) { return gammaCameraGetViewport; }
      if (gammaEquals(signature, "viewport=(_)")) { return gammaCameraSetViewport; }
    }
  }

  if (gammaEquals(class_name, "Renderer")) {
    if (kind == AGATE_FOREIGN_METHOD_INSTANCE) {
      if (gammaEquals(signature, "init new(_)")) { return gammaRendererNew; }
      if (gammaEquals(signature, "clear()")) { return gammaRendererClear0; }
      if (gammaEquals(signature, "clear(_)")) { return gammaRendererClear1; }
      if (gammaEquals(signature, "display()")) { return gammaRendererDisplay; }
      if (gammaEquals(signature, "camera=(_)")) { return gammaRendererSetCamera; }
      if (gammaEquals(signature, "world_to_device(_)")) { return gammaRendererWorldToDevice1; }
      if (gammaEquals(signature, "world_to_device(_,_)")) { return gammaRendererWorldToDevice2; }
      if (gammaEquals(signature, "device_to_world(_)")) { return gammaRendererDeviceToWorld1; }
      if (gammaEquals(signature, "device_to_world(_,_)")) { return gammaRendererDeviceToWorld2; }
      if (gammaEquals(signature, "draw_object(_,_)")) { return gammaRendererDrawObject; }
      if (gammaEquals(signature, "draw_rect(_,_)")) { return gammaRendererDrawRect2; }
      if (gammaEquals(signature, "vsynced")) { return gammaRendererIsVsynced; }
      if (gammaEquals(signature, "vsynced=(_)")) { return gammaRendererSetVsynced; }
    }
  }

  if (gammaEquals(class_name, "Transform")) {
    if (kind == AGATE_FOREIGN_METHOD_INSTANCE) {
      if (gammaEquals(signature, "init new()")) { return gammaTransformNew0; }
      if (gammaEquals(signature, "init new(_)")) { return gammaTransformNew1; }
      if (gammaEquals(signature, "init new(_,_)")) { return gammaTransformNew2; }
      if (gammaEquals(signature, "init new(_,_,_)")) { return gammaTransformNew3; }
      if (gammaEquals(signature, "origin")) { return gammaTransformGetOrigin; }
      if (gammaEquals(signature, "origin=(_)")) { return gammaTransformSetOrigin; }
      if (gammaEquals(signature, "position")) { return gammaTransformGetPosition; }
      if (gammaEquals(signature, "position=(_)")) { return gammaTransformSetPosition; }
      if (gammaEquals(signature, "move(_)")) { return gammaTransformMove; }
      if (gammaEquals(signature, "rotation")) { return gammaTransformGetRotation; }
      if (gammaEquals(signature, "rotation=(_)")) { return gammaTransformSetRotation; }
      if (gammaEquals(signature, "rotate(_)")) { return gammaTransformRotate; }
      if (gammaEquals(signature, "scale")) { return gammaTransformGetScale; }
      if (gammaEquals(signature, "scale=(_)")) { return gammaTransformSetScale; }
      if (gammaEquals(signature, "scale(_)")) { return gammaTransformScale; }
    }
  }

  return NULL;
}
