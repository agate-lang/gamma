#include "gamma_root.h"

#include <assert.h>
#include <float.h>
#include <math.h>

#include <stdio.h>

#include "gamma_common.h"
#include "gamma_tags.h"

/*
 * Vec2F
 */

// class

static ptrdiff_t gammaVec2FAllocate(AgateVM *vm, const char *unit_name, const char *class_name) {
  return sizeof(struct GammaVec2F);
}

static uint64_t gammaVec2FTag(AgateVM *vm, const char *unit_name, const char *class_name) {
  return GAMMA_VEC2F_TAG;
}

// methods

static void gammaVec2FZero(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_VEC2F_TAG);
  struct GammaVec2F *vec = agateSlotGetForeign(vm, 0);
  vec->x = 0.0f;
  vec->y = 0.0f;
}

static void gammaVec2FNew(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_VEC2F_TAG);
  struct GammaVec2F *vec = agateSlotGetForeign(vm, 0);
  vec->x = (float) agateSlotGetFloat(vm, 1);
  vec->y = (float) agateSlotGetFloat(vm, 2);
}

static void gammaVec2FUnit(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_VEC2F_TAG);
  struct GammaVec2F *vec = agateSlotGetForeign(vm, 0);
  float angle = (float) agateSlotGetFloat(vm, 1);
  vec->x = cosf(angle);
  vec->y = sinf(angle);
}

static void gammaVec2FGetX(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_VEC2F_TAG);
  struct GammaVec2F *vec = agateSlotGetForeign(vm, 0);
  agateSlotSetFloat(vm, AGATE_RETURN_SLOT, vec->x);
}

static void gammaVec2FGetY(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_VEC2F_TAG);
  struct GammaVec2F *vec = agateSlotGetForeign(vm, 0);
  agateSlotSetFloat(vm, AGATE_RETURN_SLOT, vec->y);
}

static void gammaVec2FSetX(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_VEC2F_TAG);
  struct GammaVec2F *vec = agateSlotGetForeign(vm, 0);
  vec->x = (float) agateSlotGetFloat(vm, 1);
}

static void gammaVec2FSetY(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_VEC2F_TAG);
  struct GammaVec2F *vec = agateSlotGetForeign(vm, 0);
  vec->y = (float) agateSlotGetFloat(vm, 1);
}

static void gammaVec2FPlus(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_VEC2F_TAG);
  struct GammaVec2F *vec = agateSlotGetForeign(vm, 0);

  ptrdiff_t class_slot = agateSlotAllocate(vm);
  agateGetVariable(vm, "gamma", "Vec2F", class_slot);
  struct GammaVec2F *result = agateSlotSetForeign(vm, AGATE_RETURN_SLOT, class_slot);

  result->x = vec->x;
  result->y = vec->y;
}

static void gammaVec2FMinus(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_VEC2F_TAG);
  struct GammaVec2F *vec = agateSlotGetForeign(vm, 0);

  ptrdiff_t class_slot = agateSlotAllocate(vm);
  agateGetVariable(vm, "gamma", "Vec2F", class_slot);
  struct GammaVec2F *result = agateSlotSetForeign(vm, AGATE_RETURN_SLOT, class_slot);

  result->x = - vec->x;
  result->y = - vec->y;
}

#define GAMMA_VEC2F_OP(name, op)                                \
static void gammaVec2F ## name(AgateVM *vm) {                   \
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_VEC2F_TAG);     \
  struct GammaVec2F *vec = agateSlotGetForeign(vm, 0);          \
  ptrdiff_t class_slot = agateSlotAllocate(vm);                 \
  agateGetVariable(vm, "gamma", "Vec2F", class_slot);           \
  struct GammaVec2F *result = agateSlotSetForeign(vm, AGATE_RETURN_SLOT, class_slot); \
  if (agateSlotType(vm, 1) == AGATE_TYPE_FLOAT) {               \
    float value = (float) agateSlotGetFloat(vm, 1);             \
    result->x = vec->x op value;                                \
    result->y = vec->y op value;                                \
  } else if (agateSlotType(vm, 1) == AGATE_TYPE_FOREIGN && agateSlotGetForeignTag(vm, 0) == GAMMA_VEC2F_TAG) {  \
    struct GammaVec2F *other = agateSlotGetForeign(vm, 1);      \
    result->x = vec->x op other->x;                             \
    result->y = vec->y op other->y;                             \
  }                                                             \
}

GAMMA_VEC2F_OP(Add, +)
GAMMA_VEC2F_OP(Sub, -)
GAMMA_VEC2F_OP(Mul, *)
GAMMA_VEC2F_OP(Div, /)

static void gammaVec2FEq(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_VEC2F_TAG);
  struct GammaVec2F *vec = agateSlotGetForeign(vm, 0);

  assert(agateSlotGetForeignTag(vm, 1) == GAMMA_VEC2F_TAG); // TODO: error
  struct GammaVec2F *other = agateSlotGetForeign(vm, 1);

  agateSlotSetBool(vm, AGATE_RETURN_SLOT, vec->x == other->x && vec->y == other->y);
}

static void gammaVec2FNotEq(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_VEC2F_TAG);
  struct GammaVec2F *vec = agateSlotGetForeign(vm, 0);

  assert(agateSlotGetForeignTag(vm, 1) == GAMMA_VEC2F_TAG); // TODO: error
  struct GammaVec2F *other = agateSlotGetForeign(vm, 1);

  agateSlotSetBool(vm, AGATE_RETURN_SLOT, vec->x != other->x || vec->y != other->y);
}


/*
 * Vec2I
 */

// class

static ptrdiff_t gammaVec2IAllocate(AgateVM *vm, const char *unit_name, const char *class_name) {
  return sizeof(struct GammaVec2I);
}

static uint64_t gammaVec2ITag(AgateVM *vm, const char *unit_name, const char *class_name) {
  return GAMMA_VEC2I_TAG;
}

// methods

static void gammaVec2IZero(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_VEC2I_TAG);
  struct GammaVec2I *vec = agateSlotGetForeign(vm, 0);
  vec->x = 0;
  vec->y = 0;
}

static void gammaVec2INew(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_VEC2I_TAG);
  struct GammaVec2I *vec = agateSlotGetForeign(vm, 0);
  vec->x = (int) agateSlotGetInt(vm, 1);
  vec->y = (int) agateSlotGetInt(vm, 2);
}

static void gammaVec2IGetX(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_VEC2I_TAG);
  struct GammaVec2I *vec = agateSlotGetForeign(vm, 0);
  agateSlotSetInt(vm, AGATE_RETURN_SLOT, vec->x);
}

static void gammaVec2IGetY(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_VEC2I_TAG);
  struct GammaVec2I *vec = agateSlotGetForeign(vm, 0);
  agateSlotSetInt(vm, AGATE_RETURN_SLOT, vec->y);
}

static void gammaVec2ISetX(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_VEC2I_TAG);
  struct GammaVec2I *vec = agateSlotGetForeign(vm, 0);
  vec->x = (int) agateSlotGetInt(vm, 1);
}

static void gammaVec2ISetY(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_VEC2I_TAG);
  struct GammaVec2I *vec = agateSlotGetForeign(vm, 0);
  vec->y = (int) agateSlotGetInt(vm, 1);
}

static void gammaVec2IPlus(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_VEC2I_TAG);
  struct GammaVec2I *vec = agateSlotGetForeign(vm, 0);

  ptrdiff_t class_slot = agateSlotAllocate(vm);
  agateGetVariable(vm, "gamma", "Vec2I", class_slot);
  struct GammaVec2I *result = agateSlotSetForeign(vm, AGATE_RETURN_SLOT, class_slot);

  result->x = vec->x;
  result->y = vec->y;
}

static void gammaVec2IMinus(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_VEC2I_TAG);
  struct GammaVec2I *vec = agateSlotGetForeign(vm, 0);

  ptrdiff_t class_slot = agateSlotAllocate(vm);
  agateGetVariable(vm, "gamma", "Vec2I", class_slot);
  struct GammaVec2I *result = agateSlotSetForeign(vm, AGATE_RETURN_SLOT, class_slot);

  result->x = - vec->x;
  result->y = - vec->y;
}

#define GAMMA_VEC2I_OP(name, op)                                \
static void gammaVec2I ## name(AgateVM *vm) {                   \
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_VEC2I_TAG);     \
  struct GammaVec2I *vec = agateSlotGetForeign(vm, 0);          \
  ptrdiff_t class_slot = agateSlotAllocate(vm);                 \
  agateGetVariable(vm, "gamma", "Vec2I", class_slot);           \
  struct GammaVec2I *result = agateSlotSetForeign(vm, AGATE_RETURN_SLOT, class_slot); \
  if (agateSlotType(vm, 1) == AGATE_TYPE_FLOAT) {               \
    int value = (int) agateSlotGetInt(vm, 1);                   \
    result->x = vec->x op value;                                \
    result->y = vec->y op value;                                \
  } else if (agateSlotType(vm, 1) == AGATE_TYPE_FOREIGN && agateSlotGetForeignTag(vm, 0) == GAMMA_VEC2I_TAG) {  \
    struct GammaVec2I *other = agateSlotGetForeign(vm, 1);      \
    result->x = vec->x op other->x;                             \
    result->y = vec->y op other->y;                             \
  }                                                             \
}

GAMMA_VEC2I_OP(Add, +)
GAMMA_VEC2I_OP(Sub, -)
GAMMA_VEC2I_OP(Mul, *)
GAMMA_VEC2I_OP(Div, /)

static void gammaVec2IEq(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_VEC2I_TAG);
  struct GammaVec2I *vec = agateSlotGetForeign(vm, 0);

  assert(agateSlotGetForeignTag(vm, 1) == GAMMA_VEC2I_TAG); // TODO: error
  struct GammaVec2I *other = agateSlotGetForeign(vm, 1);

  agateSlotSetBool(vm, AGATE_RETURN_SLOT, vec->x == other->x && vec->y == other->y);
}

static void gammaVec2INotEq(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_VEC2I_TAG);
  struct GammaVec2I *vec = agateSlotGetForeign(vm, 0);

  assert(agateSlotGetForeignTag(vm, 1) == GAMMA_VEC2I_TAG); // TODO: error
  struct GammaVec2I *other = agateSlotGetForeign(vm, 1);

  agateSlotSetBool(vm, AGATE_RETURN_SLOT, vec->x != other->x || vec->y != other->y);
}


/*
 * Color
 */

// class

static ptrdiff_t gammaColorAllocate(AgateVM *vm, const char *unit_name, const char *class_name) {
  return sizeof(struct GammaColor);
}

static uint64_t gammaColorTag(AgateVM *vm, const char *unit_name, const char *class_name) {
  return GAMMA_COLOR_TAG;
}

// methods

static void gammaColorNew(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_COLOR_TAG);
  struct GammaColor *color = agateSlotGetForeign(vm, 0);

  color->r = gammaClampF((float) agateSlotGetFloat(vm, 1), 0.0f, 1.0f);
  color->g = gammaClampF((float) agateSlotGetFloat(vm, 2), 0.0f, 1.0f);
  color->b = gammaClampF((float) agateSlotGetFloat(vm, 3), 0.0f, 1.0f);
  color->a = gammaClampF((float) agateSlotGetFloat(vm, 4), 0.0f, 1.0f);
}

static void gammaColorRgba32(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_COLOR_TAG);
  struct GammaColor *color = agateSlotGetForeign(vm, 0);

  int64_t rgba = agateSlotGetInt(vm, 1);

  color->r = ((rgba >>  0) & 0xFF) / 255.0f;
  color->g = ((rgba >>  8) & 0xFF) / 255.0f;
  color->b = ((rgba >> 16) & 0xFF) / 255.0f;
  color->a = ((rgba >> 24) & 0xFF) / 255.0f;
}

#define GAMMA_COLOR_GET(name, field)                        \
static void gammaColorGet ## name(AgateVM *vm) {            \
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_COLOR_TAG); \
  struct GammaColor *color = agateSlotGetForeign(vm, 0);    \
  agateSlotSetFloat(vm, AGATE_RETURN_SLOT, color->field);   \
}

GAMMA_COLOR_GET(R, r)
GAMMA_COLOR_GET(G, g)
GAMMA_COLOR_GET(B, b)
GAMMA_COLOR_GET(A, a)

#define GAMMA_COLOR_SET(name, field)                        \
static void gammaColorSet ## name(AgateVM *vm) {            \
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_COLOR_TAG); \
  struct GammaColor *color = agateSlotGetForeign(vm, 0);    \
  color->field = (float) agateSlotGetFloat(vm, 1);          \
}

GAMMA_COLOR_SET(R, r)
GAMMA_COLOR_SET(G, g)
GAMMA_COLOR_SET(B, b)
GAMMA_COLOR_SET(A, a)

#define GAMMA_COLOR_OP(name, op)                                \
static void gammaColor ## name(AgateVM *vm) {                   \
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_COLOR_TAG);     \
  struct GammaColor *color = agateSlotGetForeign(vm, 0);        \
  ptrdiff_t class_slot = agateSlotAllocate(vm);                 \
  agateGetVariable(vm, "gamma", "Color", class_slot);           \
  struct GammaColor *result = agateSlotSetForeign(vm, AGATE_RETURN_SLOT, class_slot); \
  if (agateSlotType(vm, 1) == AGATE_TYPE_FLOAT) {               \
    float value = (float) agateSlotGetFloat(vm, 1);             \
    result->r = gammaClampF(color->r op value, 0.0f, 1.0f);     \
    result->g = gammaClampF(color->g op value, 0.0f, 1.0f);     \
    result->b = gammaClampF(color->b op value, 0.0f, 1.0f);     \
    result->a = gammaClampF(color->a op value, 0.0f, 1.0f);     \
  } else if (agateSlotType(vm, 1) == AGATE_TYPE_FOREIGN && agateSlotGetForeignTag(vm, 0) == GAMMA_COLOR_TAG) {  \
    struct GammaColor *other = agateSlotGetForeign(vm, 1);      \
    result->r = gammaClampF(color->r op other->r, 0.0f, 1.0f);  \
    result->g = gammaClampF(color->g op other->g, 0.0f, 1.0f);  \
    result->b = gammaClampF(color->b op other->b, 0.0f, 1.0f);  \
    result->a = gammaClampF(color->a op other->a, 0.0f, 1.0f);  \
  }                                                             \
}
// TODO: error in the last case

GAMMA_COLOR_OP(Add, +)
GAMMA_COLOR_OP(Sub, -)
GAMMA_COLOR_OP(Mul, *)
GAMMA_COLOR_OP(Div, /)

static void gammaColorEq(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_COLOR_TAG);
  struct GammaColor *color = agateSlotGetForeign(vm, 0);

  assert(agateSlotGetForeignTag(vm, 1) == GAMMA_COLOR_TAG); // TODO: error
  struct GammaColor *other = agateSlotGetForeign(vm, 1);

  agateSlotSetBool(vm, AGATE_RETURN_SLOT, color->r == other->r && color->g == other->g && color->b == other->b && color->a == other->a);
}

static void gammaColorNotEq(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_COLOR_TAG);
  struct GammaColor *color = agateSlotGetForeign(vm, 0);

  assert(agateSlotGetForeignTag(vm, 1) == GAMMA_COLOR_TAG); // TODO: error
  struct GammaColor *other = agateSlotGetForeign(vm, 1);

  agateSlotSetBool(vm, AGATE_RETURN_SLOT, color->r != other->r || color->g != other->g || color->b != other->b || color->a != other->a);
}

struct HSV {
  float h;
  float s;
  float v;
  float a;
};

static void gammaConvertRgbToHsv(struct HSV *hsv, const struct GammaColor *color) {
  const float r = color->r;
  const float g = color->g;
  const float b = color->b;

  const float min = gammaMin3F(r, g, b);
  const float max = gammaMax3F(r, g, b);

  if ((max - min) > FLT_EPSILON) {
    if (max == r) {
      hsv->h = fmodf(60.0f * (g - b) / (max - min) + 360.0f, 360.0f);
    } else if (max == g) {
      hsv->h = 60.0f * (b - r) / (max - min) + 120.0f;
    } else if (max == b) {
      hsv->h = 60.0f * (r - g) / (max - min) + 240.0f;
    } else {
      assert(false);
    }
  } else {
    hsv->h = 0.0f;
  }

  hsv->s = (max < FLT_EPSILON ? 0.0f : (1.0f - min / max));
  hsv->v = max;
  hsv->a = color->a;
}

static void gammaConvertHsvToRgb(struct GammaColor *color, const struct HSV *hsv) {
  const float h = hsv->h / 60.0f;
  const float s = hsv->s;
  const float v = hsv->v;

  const int i = ((int) h) % 6;
  assert(0 <= i && i < 6);

  const float f = h - (float) i;
  const float x = v * (1.0f - s);
  const float y = v * (1.0f - (f * s));
  const float z = v * (1.0f - (1.0f - f) * s);

  switch (i) {
    case 0: color->r = v; color->g = z; color->b = x; break;
    case 1: color->r = y; color->g = v; color->b = x; break;
    case 2: color->r = x; color->g = v; color->b = z; break;
    case 3: color->r = x; color->g = y; color->b = v; break;
    case 4: color->r = z; color->g = x; color->b = v; break;
    case 5: color->r = v; color->g = x; color->b = y; break;
    default: assert(false); break;
  }

  color->a = hsv->a;
}

static void gammaColorDarker(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_COLOR_TAG);
  struct GammaColor *color = agateSlotGetForeign(vm, 0);
  float percent = (float) agateSlotGetFloat(vm, 1);

  ptrdiff_t class_slot = agateSlotAllocate(vm);
  agateGetVariable(vm, "gamma", "Color", class_slot);
  struct GammaColor *result = agateSlotSetForeign(vm, AGATE_RETURN_SLOT, class_slot);

  struct HSV hsv;
  gammaConvertRgbToHsv(&hsv, color);
  hsv.v -= hsv.v * percent;
  gammaConvertHsvToRgb(result, &hsv);
}

static void gammaColorLighter(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_COLOR_TAG);
  struct GammaColor *color = agateSlotGetForeign(vm, 0);
  float percent = (float) agateSlotGetFloat(vm, 1);

  ptrdiff_t class_slot = agateSlotAllocate(vm);
  agateGetVariable(vm, "gamma", "Color", class_slot);
  struct GammaColor *result = agateSlotSetForeign(vm, AGATE_RETURN_SLOT, class_slot);

  struct HSV hsv;
  gammaConvertRgbToHsv(&hsv, color);
  hsv.v += hsv.v * percent;

  if (hsv.v > 1) {
    hsv.s -= (hsv.v - 1);

    if (hsv.s < 0) {
      hsv.s = 0;
    }

    hsv.v = 1;
  }

  gammaConvertHsvToRgb(result, &hsv);
}

/*
 * RectF
 */

// class

static ptrdiff_t gammaRectFAllocate(AgateVM *vm, const char *unit_name, const char *class_name) {
  return sizeof(struct GammaRectF);
}

static uint64_t gammaRectFTag(AgateVM *vm, const char *unit_name, const char *class_name) {
  return GAMMA_RECTF_TAG;
}

// methods

static void gammaRectFNew4(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_RECTF_TAG);
  struct GammaRectF *rect = agateSlotGetForeign(vm, 0);

  rect->x = (float) agateSlotGetFloat(vm, 1);
  rect->y = (float) agateSlotGetFloat(vm, 2);
  rect->w = (float) agateSlotGetFloat(vm, 3);
  rect->h = (float) agateSlotGetFloat(vm, 4);
}

static void gammaRectFNew2(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_RECTF_TAG);
  struct GammaRectF *rect = agateSlotGetForeign(vm, 0);

  assert(agateSlotGetForeignTag(vm, 1) == GAMMA_VEC2F_TAG);
  struct GammaVec2F *position = agateSlotGetForeign(vm, 1);

  assert(agateSlotGetForeignTag(vm, 2) == GAMMA_VEC2F_TAG);
  struct GammaVec2F *size = agateSlotGetForeign(vm, 2);

  rect->x = position->x;
  rect->y = position->y;
  rect->w = size->x;
  rect->h = size->y;
}

static void gammaRectFNew1(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_RECTF_TAG);
  struct GammaRectF *rect = agateSlotGetForeign(vm, 0);

  assert(agateSlotGetForeignTag(vm, 1) == GAMMA_VEC2F_TAG);
  struct GammaVec2F *size = agateSlotGetForeign(vm, 1);

  rect->x = 0.0f;
  rect->y = 0.0f;
  rect->w = size->x;
  rect->h = size->y;
}

#define GAMMA_RECTF_GET(name, field)                        \
static void gammaRectFGet ## name(AgateVM *vm) {            \
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_RECTF_TAG); \
  struct GammaRectF *rect = agateSlotGetForeign(vm, 0);     \
  agateSlotSetFloat(vm, AGATE_RETURN_SLOT, rect->field);    \
}

GAMMA_RECTF_GET(X, x)
GAMMA_RECTF_GET(Y, y)
GAMMA_RECTF_GET(W, w)
GAMMA_RECTF_GET(H, h)

#define GAMMA_RECTF_SET(name, field)                        \
static void gammaRectFSet ## name(AgateVM *vm) {            \
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_RECTF_TAG); \
  struct GammaRectF *rect = agateSlotGetForeign(vm, 0);     \
  rect->field = (float) agateSlotGetFloat(vm, 1);           \
}

GAMMA_RECTF_SET(X, x)
GAMMA_RECTF_SET(Y, y)
GAMMA_RECTF_SET(W, w)
GAMMA_RECTF_SET(H, h)

static void gammaRectFPosition(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_RECTF_TAG);
  struct GammaRectF *rect = agateSlotGetForeign(vm, 0);

  ptrdiff_t class_slot = agateSlotAllocate(vm);
  agateGetVariable(vm, "gamma", "Vec2F", class_slot);
  struct GammaVec2F *result = agateSlotSetForeign(vm, AGATE_RETURN_SLOT, class_slot);

  result->x = rect->x;
  result->y = rect->y;
}

static void gammaRectFSize(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_RECTF_TAG);
  struct GammaRectF *rect = agateSlotGetForeign(vm, 0);

  ptrdiff_t class_slot = agateSlotAllocate(vm);
  agateGetVariable(vm, "gamma", "Vec2F", class_slot);
  struct GammaVec2F *result = agateSlotSetForeign(vm, AGATE_RETURN_SLOT, class_slot);

  result->x = rect->w;
  result->y = rect->h;
}

static void gammaRectFContains2(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_RECTF_TAG);
  struct GammaRectF *rect = agateSlotGetForeign(vm, 0);

  float x = (float) agateSlotGetFloat(vm, 1);
  float y = (float) agateSlotGetFloat(vm, 2);

  agateSlotSetBool(vm, AGATE_RETURN_SLOT, rect->x <= x && x < rect->x + rect->w && rect->y <= y && y <= rect->y + rect->h);
}

static void gammaRectFContains1(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_RECTF_TAG);
  struct GammaRectF *rect = agateSlotGetForeign(vm, 0);

  if (agateSlotType(vm, 1) == AGATE_TYPE_FOREIGN) {
    switch (agateSlotGetForeignTag(vm, 1)) {
      case GAMMA_VEC2F_TAG:
      {
        struct GammaVec2F *vec = agateSlotGetForeign(vm, 1);
        agateSlotSetBool(vm, AGATE_RETURN_SLOT, rect->x <= vec->x && vec->x < rect->x + rect->w && rect->y <= vec->y && vec->y <= rect->y + rect->h);
        break;
      }

      case GAMMA_RECTF_TAG:
      {
        struct GammaRectF *other = agateSlotGetForeign(vm, 1);
        agateSlotSetBool(vm, AGATE_RETURN_SLOT, rect->x <= other->x && other->x + + other->w <= rect->x + rect->w && rect->y <= other->y && other->y + other->h <= rect->y + rect->h);
        break;
      }

      default:
        // TODO: error
        agateSlotSetNil(vm, AGATE_RETURN_SLOT);
        break;
    }
  }
}

static void gammaRectFIntersects(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_RECTF_TAG);
  struct GammaRectF *rect = agateSlotGetForeign(vm, 0);

  if (agateSlotType(vm, 1) == AGATE_TYPE_FOREIGN && agateSlotGetForeignTag(vm, 1) == GAMMA_RECTF_TAG) {
    struct GammaRectF *other = agateSlotGetForeign(vm, 1);
    agateSlotSetBool(vm, AGATE_RETURN_SLOT, rect->x < other->x + other->w && other->x < rect->x + rect->w && rect->y < other->y + other->h && other->y < rect->y + rect->h);
  }
}


/*
 * RectI
 */

// class

static ptrdiff_t gammaRectIAllocate(AgateVM *vm, const char *unit_name, const char *class_name) {
  return sizeof(struct GammaRectI);
}

static uint64_t gammaRectITag(AgateVM *vm, const char *unit_name, const char *class_name) {
  return GAMMA_RECTI_TAG;
}

// methods

static void gammaRectINew4(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_RECTI_TAG);
  struct GammaRectI *rect = agateSlotGetForeign(vm, 0);

  rect->x = (int) agateSlotGetInt(vm, 1);
  rect->y = (int) agateSlotGetInt(vm, 2);
  rect->w = (int) agateSlotGetInt(vm, 3);
  rect->h = (int) agateSlotGetInt(vm, 4);
}

static void gammaRectINew2(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_RECTI_TAG);
  struct GammaRectI *rect = agateSlotGetForeign(vm, 0);

  assert(agateSlotGetForeignTag(vm, 1) == GAMMA_VEC2I_TAG);
  struct GammaVec2I *position = agateSlotGetForeign(vm, 1);

  assert(agateSlotGetForeignTag(vm, 2) == GAMMA_VEC2I_TAG);
  struct GammaVec2I *size = agateSlotGetForeign(vm, 2);

  rect->x = position->x;
  rect->y = position->y;
  rect->w = size->x;
  rect->h = size->y;
}

static void gammaRectINew1(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_RECTI_TAG);
  struct GammaRectI *rect = agateSlotGetForeign(vm, 0);

  assert(agateSlotGetForeignTag(vm, 1) == GAMMA_VEC2I_TAG);
  struct GammaVec2I *size = agateSlotGetForeign(vm, 1);

  rect->x = 0;
  rect->y = 0;
  rect->w = size->x;
  rect->h = size->y;
}

#define GAMMA_RECTI_GET(name, field)                        \
static void gammaRectIGet ## name(AgateVM *vm) {            \
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_RECTI_TAG); \
  struct GammaRectI *rect = agateSlotGetForeign(vm, 0);     \
  agateSlotSetInt(vm, AGATE_RETURN_SLOT, rect->field);      \
}

GAMMA_RECTI_GET(X, x)
GAMMA_RECTI_GET(Y, y)
GAMMA_RECTI_GET(W, w)
GAMMA_RECTI_GET(H, h)

#define GAMMA_RECTI_SET(name, field)                        \
static void gammaRectISet ## name(AgateVM *vm) {            \
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_RECTI_TAG); \
  struct GammaRectI *rect = agateSlotGetForeign(vm, 0);     \
  rect->field = (int) agateSlotGetFloat(vm, 1);             \
}

GAMMA_RECTI_SET(X, x)
GAMMA_RECTI_SET(Y, y)
GAMMA_RECTI_SET(W, w)
GAMMA_RECTI_SET(H, h)

static void gammaRectIPosition(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_RECTI_TAG);
  struct GammaRectI *rect = agateSlotGetForeign(vm, 0);

  ptrdiff_t class_slot = agateSlotAllocate(vm);
  agateGetVariable(vm, "gamma", "Vec2I", class_slot);
  struct GammaVec2I *result = agateSlotSetForeign(vm, AGATE_RETURN_SLOT, class_slot);

  result->x = rect->x;
  result->y = rect->y;
}

static void gammaRectISize(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_RECTI_TAG);
  struct GammaRectI *rect = agateSlotGetForeign(vm, 0);

  ptrdiff_t class_slot = agateSlotAllocate(vm);
  agateGetVariable(vm, "gamma", "Vec2I", class_slot);
  struct GammaVec2I *result = agateSlotSetForeign(vm, AGATE_RETURN_SLOT, class_slot);

  result->x = rect->w;
  result->y = rect->h;
}

static void gammaRectIContains2(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_RECTI_TAG);
  struct GammaRectI *rect = agateSlotGetForeign(vm, 0);

  int x = (int) agateSlotGetFloat(vm, 1);
  int y = (int) agateSlotGetFloat(vm, 2);

  agateSlotSetBool(vm, AGATE_RETURN_SLOT, rect->x <= x && x < rect->x + rect->w && rect->y <= y && y <= rect->y + rect->h);
}

static void gammaRectIContains1(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_RECTI_TAG);
  struct GammaRectI *rect = agateSlotGetForeign(vm, 0);

  if (agateSlotType(vm, 1) == AGATE_TYPE_FOREIGN) {
    switch (agateSlotGetForeignTag(vm, 1)) {
      case GAMMA_VEC2I_TAG:
      {
        struct GammaVec2I *vec = agateSlotGetForeign(vm, 1);
        agateSlotSetBool(vm, AGATE_RETURN_SLOT, rect->x <= vec->x && vec->x < rect->x + rect->w && rect->y <= vec->y && vec->y <= rect->y + rect->h);
        break;
      }

      case GAMMA_RECTI_TAG:
      {
        struct GammaRectI *other = agateSlotGetForeign(vm, 1);
        agateSlotSetBool(vm, AGATE_RETURN_SLOT, rect->x <= other->x && other->x + + other->w <= rect->x + rect->w && rect->y <= other->y && other->y + other->h <= rect->y + rect->h);
        break;
      }

      default:
        // TODO: error
        agateSlotSetNil(vm, AGATE_RETURN_SLOT);
        break;
    }
  }
}

static void gammaRectIIntersects(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_RECTI_TAG);
  struct GammaRectI *rect = agateSlotGetForeign(vm, 0);

  if (agateSlotType(vm, 1) == AGATE_TYPE_FOREIGN && agateSlotGetForeignTag(vm, 1) == GAMMA_RECTI_TAG) {
    struct GammaRectI *other = agateSlotGetForeign(vm, 1);
    agateSlotSetBool(vm, AGATE_RETURN_SLOT, rect->x < other->x + other->w && other->x < rect->x + rect->w && rect->y < other->y + other->h && other->y < rect->y + rect->h);
  }
}

/*
 * Mat3F
 */

// class

static ptrdiff_t gammaMat3FAllocate(AgateVM *vm, const char *unit_name, const char *class_name) {
  return sizeof(struct GammaMat3F);
}

static uint64_t gammaMat3FTag(AgateVM *vm, const char *unit_name, const char *class_name) {
  return GAMMA_MAT3F_TAG;
}

// methods

static void gammaMat3FIdentity(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_MAT3F_TAG);
  struct GammaMat3F *mat = agateSlotGetForeign(vm, 0);

  mat->data[0][0] = 1.0; mat->data[1][0] = 0.0; mat->data[2][0] = 0.0;
  mat->data[0][1] = 0.0; mat->data[1][1] = 1.0; mat->data[2][1] = 0.0;
  mat->data[0][2] = 0.0; mat->data[1][2] = 0.0; mat->data[2][2] = 1.0;
}

static void gammaMat3FTranslation(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_MAT3F_TAG);
  struct GammaMat3F *mat = agateSlotGetForeign(vm, 0);

  float tx = (float) agateSlotGetFloat(vm, 1);
  float ty = (float) agateSlotGetFloat(vm, 2);

  mat->data[0][0] = 1.0; mat->data[1][0] = 0.0; mat->data[2][0] = tx;
  mat->data[0][1] = 0.0; mat->data[1][1] = 1.0; mat->data[2][1] = ty;
  mat->data[0][2] = 0.0; mat->data[1][2] = 0.0; mat->data[2][2] = 1.0;
}

static void gammaMat3FRotation1(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_MAT3F_TAG);
  struct GammaMat3F *mat = agateSlotGetForeign(vm, 0);

  float angle = (float) agateSlotGetFloat(vm, 1);
  float c = cosf(angle);
  float s = sinf(angle);

  mat->data[0][0] =  c ; mat->data[1][0] = -s ; mat->data[2][0] = 0.0;
  mat->data[0][1] =  s ; mat->data[1][1] =  c ; mat->data[2][1] = 0.0;
  mat->data[0][2] = 0.0; mat->data[1][2] = 0.0; mat->data[2][2] = 1.0;
}

static void gammaMat3FRotation2(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_MAT3F_TAG);
  struct GammaMat3F *mat = agateSlotGetForeign(vm, 0);

  float angle = (float) agateSlotGetFloat(vm, 1);

  assert(agateSlotGetForeignTag(vm, 2) == GAMMA_VEC2F_TAG);
  struct GammaVec2F *center = agateSlotGetForeign(vm, 2);

  float c = cosf(angle);
  float s = sinf(angle);

  mat->data[0][0] =  c ; mat->data[1][0] = -s ; mat->data[2][0] = center->x * (1 - c) + center->y * s;
  mat->data[0][1] =  s ; mat->data[1][1] =  c ; mat->data[2][1] = center->y * (1 - c) - center->y * s;
  mat->data[0][2] = 0.0; mat->data[1][2] = 0.0; mat->data[2][2] = 1.0;
}

static void gammaMat3FScale(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_MAT3F_TAG);
  struct GammaMat3F *mat = agateSlotGetForeign(vm, 0);

  float sx = (float) agateSlotGetFloat(vm, 1);
  float sy = (float) agateSlotGetFloat(vm, 2);

  mat->data[0][0] =  sx; mat->data[1][0] = 0.0; mat->data[2][0] = 0.0;
  mat->data[0][1] = 0.0; mat->data[1][1] =  sy; mat->data[2][1] = 0.0;
  mat->data[0][2] = 0.0; mat->data[1][2] = 0.0; mat->data[2][2] = 1.0;
}

static void gammaMat3FSubscript(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_MAT3F_TAG);
  struct GammaMat3F *mat = agateSlotGetForeign(vm, 0);

  int64_t row = agateSlotGetInt(vm, 1);
  int64_t col = agateSlotGetInt(vm, 2);
  agateSlotSetFloat(vm, AGATE_RETURN_SLOT, mat->data[col][row]);
}

static void gammaMat3FAdd(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_MAT3F_TAG);
  struct GammaMat3F *mat = agateSlotGetForeign(vm, 0);

  assert(agateSlotGetForeignTag(vm, 1) == GAMMA_MAT3F_TAG);
  struct GammaMat3F *other = agateSlotGetForeign(vm, 1);

  ptrdiff_t class_slot = agateSlotAllocate(vm);
  agateGetVariable(vm, "gamma", "Mat3F", class_slot);
  struct GammaMat3F *result = agateSlotSetForeign(vm, AGATE_RETURN_SLOT, class_slot);

  for (int col = 0; col < 3; ++col) {
    for (int row = 0; row < 3; ++row) {
      result->data[col][row] = mat->data[col][row] + other->data[col][row];
    }
  }
}

static void gammaMat3FSub(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_MAT3F_TAG);
  struct GammaMat3F *mat = agateSlotGetForeign(vm, 0);

  assert(agateSlotGetForeignTag(vm, 1) == GAMMA_MAT3F_TAG);
  struct GammaMat3F *other = agateSlotGetForeign(vm, 1);

  ptrdiff_t class_slot = agateSlotAllocate(vm);
  agateGetVariable(vm, "gamma", "Mat3F", class_slot);
  struct GammaMat3F *result = agateSlotSetForeign(vm, AGATE_RETURN_SLOT, class_slot);

  for (int col = 0; col < 3; ++col) {
    for (int row = 0; row < 3; ++row) {
      result->data[col][row] = mat->data[col][row] - other->data[col][row];
    }
  }
}

static void gammaMat3FMul(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_MAT3F_TAG);
  struct GammaMat3F *mat = agateSlotGetForeign(vm, 0);

  /* if (agateSlotGetForeignTag(vm, 1) == GAMMA_MAT3F_TAG) {
    struct GammaVec3F *vec = agateSlotGetForeign(vm, 1);

    ptrdiff_t class_slot = agateSlotAllocate(vm);
    agateGetVariable(vm, "gamma", "Vec3F", class_slot);
    struct GammaVec3F *result = agateSlotSetForeign(vm, AGATE_RETURN_SLOT, class_slot);

    result->x = mat->data[0][0] * vec->x + mat->data[1][0] * vec->y + mat->data[2][0] * vec->z;
    result->y = mat->data[0][1] * vec->x + mat->data[1][1] * vec->y + mat->data[2][1] * vec->z;
    result->z = mat->data[0][2] * vec->x + mat->data[1][2] * vec->y + mat->data[2][2] * vec->z;
  } else */
  if (agateSlotGetForeignTag(vm, 1) == GAMMA_MAT3F_TAG) {
    struct GammaMat3F *other = agateSlotGetForeign(vm, 1);

    ptrdiff_t class_slot = agateSlotAllocate(vm);
    agateGetVariable(vm, "gamma", "Mat3F", class_slot);
    struct GammaMat3F *result = agateSlotSetForeign(vm, AGATE_RETURN_SLOT, class_slot);

    for (int col = 0; col < 3; ++col) {
      for (int row = 0; row < 3; ++row) {
        float sum = 0.0f;

        for (int k = 0; k < 3; ++k) {
          sum += mat->data[k][row] * other->data[col][k];
        }

        result->data[col][row] = sum;
      }
    }
  } else {
    // TODO: error
  }
}

static void gammaMat3FTranspose(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_MAT3F_TAG);
  struct GammaMat3F *mat = agateSlotGetForeign(vm, 0);

  ptrdiff_t class_slot = agateSlotAllocate(vm);
  agateGetVariable(vm, "gamma", "Mat3F", class_slot);
  struct GammaMat3F *result = agateSlotSetForeign(vm, AGATE_RETURN_SLOT, class_slot);

  for (int col = 0; col < 3; ++col) {
    for (int row = 0; row < 3; ++row) {
      result->data[col][row] = mat->data[row][col];
    }
  }
}

static void gammaMat3FInvert(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_MAT3F_TAG);
  struct GammaMat3F *mat = agateSlotGetForeign(vm, 0);

  ptrdiff_t class_slot = agateSlotAllocate(vm);
  agateGetVariable(vm, "gamma", "Mat3F", class_slot);
  struct GammaMat3F *result = agateSlotSetForeign(vm, AGATE_RETURN_SLOT, class_slot);

  result->data[0][0] = mat->data[1][1] * mat->data[2][2] - mat->data[2][1] * mat->data[1][2];
  result->data[0][1] = - (mat->data[0][1] * mat->data[2][2] - mat->data[2][1] * mat->data[0][2]);
  result->data[0][2] = mat->data[0][1] * mat->data[1][2] - mat->data[1][1] * mat->data[0][2];
  result->data[1][0] = - (mat->data[1][0] * mat->data[2][2] - mat->data[2][0] * mat->data[1][2]);
  result->data[1][1] = mat->data[0][0] * mat->data[2][2] - mat->data[2][0] * mat->data[0][2];
  result->data[1][2] = - (mat->data[0][0] * mat->data[1][2] - mat->data[1][0] * mat->data[0][2]);
  result->data[2][0] = mat->data[1][0] * mat->data[2][1] - mat->data[2][0] * mat->data[1][1];
  result->data[2][1] = - (mat->data[0][0] * mat->data[2][1] - mat->data[2][0] * mat->data[0][1]);
  result->data[2][2] = mat->data[0][0] * mat->data[1][1] - mat->data[1][0] * mat->data[0][1];

  float det = mat->data[0][0] * result->data[0][0] + mat->data[0][1] * result->data[1][0] + mat->data[0][2] * result->data[2][0];

  if (det < FLT_EPSILON) {
    agateSlotSetNil(vm, AGATE_RETURN_SLOT);
    return;
  }

  for (int col = 0; col < 3; ++col) {
    for (int row = 0; row < 3; ++row) {
      result->data[col][row] /= det;
    }
  }
}

static void gammaMat3FTransformPoint(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_MAT3F_TAG);
  struct GammaMat3F *mat = agateSlotGetForeign(vm, 0);

  float x = (float) agateSlotGetFloat(vm, 1);
  float y = (float) agateSlotGetFloat(vm, 2);
  // z == 1.0f

  ptrdiff_t class_slot = agateSlotAllocate(vm);
  agateGetVariable(vm, "gamma", "Vec2F", class_slot);
  struct GammaVec2F *result = agateSlotSetForeign(vm, AGATE_RETURN_SLOT, class_slot);

  result->x = mat->data[0][0] * x + mat->data[1][0] * y +  mat->data[2][0];
  result->y = mat->data[0][1] * x + mat->data[1][1] * y +  mat->data[2][1];
}

static void gammaMat3FTransformVector(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_MAT3F_TAG);
  struct GammaMat3F *mat = agateSlotGetForeign(vm, 0);

  float x = (float) agateSlotGetFloat(vm, 1);
  float y = (float) agateSlotGetFloat(vm, 2);
  // z == 0.0f

  ptrdiff_t class_slot = agateSlotAllocate(vm);
  agateGetVariable(vm, "gamma", "Vec2F", class_slot);
  struct GammaVec2F *result = agateSlotSetForeign(vm, AGATE_RETURN_SLOT, class_slot);

  result->x = mat->data[0][0] * x + mat->data[1][0] * y;
  result->y = mat->data[0][1] * x + mat->data[1][1] * y;
}

/*
 * unit configuration
 */

AgateForeignClassHandler gammaRootClassHandler(AgateVM *vm, const char *unit_name, const char *class_name) {
  assert(gammaEquals(unit_name, "gamma"));
  AgateForeignClassHandler handler = { NULL, NULL, NULL };

  if (gammaEquals(class_name, "Vec2F")) {
    handler.allocate = gammaVec2FAllocate;
    handler.tag = gammaVec2FTag;
    return handler;
  }

  if (gammaEquals(class_name, "Vec2I")) {
    handler.allocate = gammaVec2IAllocate;
    handler.tag = gammaVec2ITag;
    return handler;
  }

  if (gammaEquals(class_name, "Color")) {
    handler.allocate = gammaColorAllocate;
    handler.tag = gammaColorTag;
    return handler;
  }

  if (gammaEquals(class_name, "RectF")) {
    handler.allocate = gammaRectFAllocate;
    handler.tag = gammaRectFTag;
    return handler;
  }

  if (gammaEquals(class_name, "RectI")) {
    handler.allocate = gammaRectIAllocate;
    handler.tag = gammaRectITag;
    return handler;
  }

  if (gammaEquals(class_name, "Mat3F")) {
    handler.allocate = gammaMat3FAllocate;
    handler.tag = gammaMat3FTag;
    return handler;
  }

  return handler;
}

AgateForeignMethodFunc gammaRootMethodHandler(AgateVM *vm, const char *unit_name, const char *class_name, AgateForeignMethodKind kind, const char *signature) {
  assert(gammaEquals(unit_name, "gamma"));

  if (gammaEquals(class_name, "Vec2F")) {
    if (kind == AGATE_FOREIGN_METHOD_INSTANCE) {
      if (gammaEquals(signature, "init zero()")) { return gammaVec2FZero; }
      if (gammaEquals(signature, "init new(_,_)")) { return gammaVec2FNew; }
      if (gammaEquals(signature, "init unit(_)")) { return gammaVec2FUnit; }
      if (gammaEquals(signature, "x")) { return gammaVec2FGetX; }
      if (gammaEquals(signature, "y")) { return gammaVec2FGetY; }
      if (gammaEquals(signature, "x=(_)")) { return gammaVec2FSetX; }
      if (gammaEquals(signature, "y=(_)")) { return gammaVec2FSetY; }
      if (gammaEquals(signature, "+")) { return gammaVec2FPlus; }
      if (gammaEquals(signature, "-")) { return gammaVec2FMinus; }
      if (gammaEquals(signature, "+(_)")) { return gammaVec2FAdd; }
      if (gammaEquals(signature, "-(_)")) { return gammaVec2FSub; }
      if (gammaEquals(signature, "*(_)")) { return gammaVec2FMul; }
      if (gammaEquals(signature, "/(_)")) { return gammaVec2FDiv; }
      if (gammaEquals(signature, "==(_)")) { return gammaVec2FEq; }
      if (gammaEquals(signature, "!=(_)")) { return gammaVec2FNotEq; }
    }
  }

  if (gammaEquals(class_name, "Vec2I")) {
    if (kind == AGATE_FOREIGN_METHOD_INSTANCE) {
      if (gammaEquals(signature, "init zero()")) { return gammaVec2IZero; }
      if (gammaEquals(signature, "init new(_,_)")) { return gammaVec2INew; }
      if (gammaEquals(signature, "x")) { return gammaVec2IGetX; }
      if (gammaEquals(signature, "y")) { return gammaVec2IGetY; }
      if (gammaEquals(signature, "x=(_)")) { return gammaVec2ISetX; }
      if (gammaEquals(signature, "y=(_)")) { return gammaVec2ISetY; }
      if (gammaEquals(signature, "+")) { return gammaVec2IPlus; }
      if (gammaEquals(signature, "-")) { return gammaVec2IMinus; }
      if (gammaEquals(signature, "+(_)")) { return gammaVec2IAdd; }
      if (gammaEquals(signature, "-(_)")) { return gammaVec2ISub; }
      if (gammaEquals(signature, "*(_)")) { return gammaVec2IMul; }
      if (gammaEquals(signature, "/(_)")) { return gammaVec2IDiv; }
      if (gammaEquals(signature, "==(_)")) { return gammaVec2IEq; }
      if (gammaEquals(signature, "!=(_)")) { return gammaVec2INotEq; }
    }
  }

  if (gammaEquals(class_name, "Color")) {
    if (kind == AGATE_FOREIGN_METHOD_INSTANCE) {
      if (gammaEquals(signature, "init new(_,_,_,_)")) { return gammaColorNew; }
      if (gammaEquals(signature, "init rgba32(_)")) { return gammaColorRgba32; }
      if (gammaEquals(signature, "r")) { return gammaColorGetR; }
      if (gammaEquals(signature, "g")) { return gammaColorGetG; }
      if (gammaEquals(signature, "b")) { return gammaColorGetB; }
      if (gammaEquals(signature, "a")) { return gammaColorGetA; }
      if (gammaEquals(signature, "r=(_)")) { return gammaColorSetR; }
      if (gammaEquals(signature, "g=(_)")) { return gammaColorSetG; }
      if (gammaEquals(signature, "b=(_)")) { return gammaColorSetB; }
      if (gammaEquals(signature, "a=(_)")) { return gammaColorSetA; }
      if (gammaEquals(signature, "+(_)")) { return gammaColorAdd; }
      if (gammaEquals(signature, "-(_)")) { return gammaColorSub; }
      if (gammaEquals(signature, "*(_)")) { return gammaColorMul; }
      if (gammaEquals(signature, "/(_)")) { return gammaColorDiv; }
      if (gammaEquals(signature, "==(_)")) { return gammaColorEq; }
      if (gammaEquals(signature, "!=(_)")) { return gammaColorNotEq; }
      if (gammaEquals(signature, "darker(_)")) { return gammaColorDarker; }
      if (gammaEquals(signature, "lighter(_)")) { return gammaColorLighter; }
    }
  }

  if (gammaEquals(class_name, "RectF")) {
    if (kind == AGATE_FOREIGN_METHOD_INSTANCE) {
      if (gammaEquals(signature, "init new(_,_,_,_)")) { return gammaRectFNew4; }
      if (gammaEquals(signature, "init new(_,_)")) { return gammaRectFNew2; }
      if (gammaEquals(signature, "init new(_)")) { return gammaRectFNew1; }
      if (gammaEquals(signature, "x")) { return gammaRectFGetX; }
      if (gammaEquals(signature, "y")) { return gammaRectFGetY; }
      if (gammaEquals(signature, "w")) { return gammaRectFGetW; }
      if (gammaEquals(signature, "h")) { return gammaRectFGetH; }
      if (gammaEquals(signature, "x=(_)")) { return gammaRectFSetX; }
      if (gammaEquals(signature, "y=(_)")) { return gammaRectFSetY; }
      if (gammaEquals(signature, "w=(_)")) { return gammaRectFSetW; }
      if (gammaEquals(signature, "h=(_)")) { return gammaRectFSetH; }
      if (gammaEquals(signature, "position")) { return gammaRectFPosition; }
      if (gammaEquals(signature, "size")) { return gammaRectFSize; }
      if (gammaEquals(signature, "contains(_,_)")) { return gammaRectFContains2; }
      if (gammaEquals(signature, "contains(_)")) { return gammaRectFContains1; }
      if (gammaEquals(signature, "intersects(_)")) { return gammaRectFIntersects; }
    }
  }

  if (gammaEquals(class_name, "RectI")) {
    if (kind == AGATE_FOREIGN_METHOD_INSTANCE) {
      if (gammaEquals(signature, "init new(_,_,_,_)")) { return gammaRectINew4; }
      if (gammaEquals(signature, "init new(_,_)")) { return gammaRectINew2; }
      if (gammaEquals(signature, "init new(_)")) { return gammaRectINew1; }
      if (gammaEquals(signature, "x")) { return gammaRectIGetX; }
      if (gammaEquals(signature, "y")) { return gammaRectIGetY; }
      if (gammaEquals(signature, "w")) { return gammaRectIGetW; }
      if (gammaEquals(signature, "h")) { return gammaRectIGetH; }
      if (gammaEquals(signature, "x=(_)")) { return gammaRectISetX; }
      if (gammaEquals(signature, "y=(_)")) { return gammaRectISetY; }
      if (gammaEquals(signature, "w=(_)")) { return gammaRectISetW; }
      if (gammaEquals(signature, "h=(_)")) { return gammaRectISetH; }
      if (gammaEquals(signature, "position")) { return gammaRectIPosition; }
      if (gammaEquals(signature, "size")) { return gammaRectISize; }
      if (gammaEquals(signature, "contains(_,_)")) { return gammaRectIContains2; }
      if (gammaEquals(signature, "contains(_)")) { return gammaRectIContains1; }
      if (gammaEquals(signature, "intersects(_)")) { return gammaRectIIntersects; }
    }
  }

  if (gammaEquals(class_name, "Mat3F")) {
    if (kind == AGATE_FOREIGN_METHOD_INSTANCE) {
      if (gammaEquals(signature, "init identity()")) { return gammaMat3FIdentity; }
      if (gammaEquals(signature, "init translation(_,_)")) { return gammaMat3FTranslation; }
      if (gammaEquals(signature, "init rotation(_)")) { return gammaMat3FRotation1; }
      if (gammaEquals(signature, "init rotation(_,_)")) { return gammaMat3FRotation2; }
      if (gammaEquals(signature, "init scale(_,_)")) { return gammaMat3FScale; }
      if (gammaEquals(signature, "[_,_]")) { return gammaMat3FSubscript; }
      if (gammaEquals(signature, "+(_)")) { return gammaMat3FAdd; }
      if (gammaEquals(signature, "-(_)")) { return gammaMat3FSub; }
      if (gammaEquals(signature, "*(_)")) { return gammaMat3FMul; }
      if (gammaEquals(signature, "transpose()")) { return gammaMat3FTranspose; }
      if (gammaEquals(signature, "invert()")) { return gammaMat3FInvert; }
      if (gammaEquals(signature, "transform_point(_,_)")) { return gammaMat3FTransformPoint; }
      if (gammaEquals(signature, "transform_vector(_,_)")) { return gammaMat3FTransformVector; }
    }
  }
  return NULL;
}

