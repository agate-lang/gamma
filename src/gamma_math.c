#include "gamma_math.h"

#include <assert.h>
#include <float.h>
#include <math.h>

#include <stdio.h>

#include "gamma_check.h"
#include "gamma_common.h"
#include "gamma_error.h"
#include "gamma_tags.h"
#include "gamma_utils.h"

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
  vec->v[0] = 0.0f;
  vec->v[1] = 0.0f;
}

static void gammaVec2FNew(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_VEC2F_TAG);
  struct GammaVec2F *vec = agateSlotGetForeign(vm, 0);

  if (!gammaCheckFloat(vm, 1, &vec->v[0])) {
    gammaError(vm, "Float parameter expected for `x`.");
    return;
  }

  if (!gammaCheckFloat(vm, 2, &vec->v[1])) {
    gammaError(vm, "Float parameter expected for `y`.");
    return;
  }
}

static void gammaVec2FUnit(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_VEC2F_TAG);
  struct GammaVec2F *vec = agateSlotGetForeign(vm, 0);
  float angle;

  if (!gammaCheckFloat(vm, 1, &angle)) {
    gammaError(vm, "Float parameter expected for `angle`.");
    return;
  }

  vec->v[0] = cosf(angle);
  vec->v[1] = sinf(angle);
}

static void gammaVec2FGetX(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_VEC2F_TAG);
  struct GammaVec2F *vec = agateSlotGetForeign(vm, 0);
  agateSlotSetFloat(vm, AGATE_RETURN_SLOT, vec->v[0]);
}

static void gammaVec2FGetY(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_VEC2F_TAG);
  struct GammaVec2F *vec = agateSlotGetForeign(vm, 0);
  agateSlotSetFloat(vm, AGATE_RETURN_SLOT, vec->v[1]);
}

static void gammaVec2FSetX(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_VEC2F_TAG);
  struct GammaVec2F *vec = agateSlotGetForeign(vm, 0);

  if (!gammaCheckFloat(vm, 1, &vec->v[0])) {
    gammaError(vm, "Float parameter expected for `value`.");
  }
}

static void gammaVec2FSetY(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_VEC2F_TAG);
  struct GammaVec2F *vec = agateSlotGetForeign(vm, 0);

  if (!gammaCheckFloat(vm, 1, &vec->v[1])) {
    gammaError(vm, "Float parameter expected for `value`.");
  }
}

static void gammaVec2FPlus(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_VEC2F_TAG);
  struct GammaVec2F *vec = agateSlotGetForeign(vm, 0);

  struct GammaVec2F *result = gammaForeignAllocate(vm, AGATE_RETURN_SLOT, "gamma/math", "Vec2F");
  result->v[0] = vec->v[0];
  result->v[1] = vec->v[1];
}

static void gammaVec2FMinus(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_VEC2F_TAG);
  struct GammaVec2F *vec = agateSlotGetForeign(vm, 0);

  struct GammaVec2F *result = gammaForeignAllocate(vm, AGATE_RETURN_SLOT, "gamma/math", "Vec2F");
  result->v[0] = - vec->v[0];
  result->v[1] = - vec->v[1];
}

#define GAMMA_VEC2F_OP(name, op)                                \
static void gammaVec2F ## name(AgateVM *vm) {                   \
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_VEC2F_TAG);     \
  struct GammaVec2F *vec = agateSlotGetForeign(vm, 0);          \
  struct GammaVec2F *result = gammaForeignAllocate(vm, AGATE_RETURN_SLOT, "gamma/math", "Vec2F"); \
  float value;                                                  \
  if (gammaCheckFloat(vm, 1, &value)) {                         \
    result->v[0] = vec->v[0] op value;                          \
    result->v[1] = vec->v[1] op value;                          \
    return;                                                     \
  }                                                             \
  struct GammaVec2F other;                                      \
  if (gammaCheckVec2F(vm, 1, &other)) {                         \
    result->v[0] = vec->v[0] op other.v[0];                     \
    result->v[1] = vec->v[1] op other.v[1];                     \
    return;                                                     \
  }                                                             \
  gammaError(vm, "Float or Vec2F parameter expected for `other`."); \
}

GAMMA_VEC2F_OP(Add, +)
GAMMA_VEC2F_OP(Sub, -)
GAMMA_VEC2F_OP(Mul, *)
GAMMA_VEC2F_OP(Div, /)

static void gammaVec2FEq(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_VEC2F_TAG);
  struct GammaVec2F *vec = agateSlotGetForeign(vm, 0);
  struct GammaVec2F other;

  if (gammaCheckVec2F(vm, 1, &other)) {
    gammaError(vm, "Vec2F parameter expected for `other`.");
    return;
  }

  agateSlotSetBool(vm, AGATE_RETURN_SLOT, vec->v[0] == other.v[0] && vec->v[1] == other.v[1]);
}

static void gammaVec2FNotEq(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_VEC2F_TAG);
  struct GammaVec2F *vec = agateSlotGetForeign(vm, 0);
  struct GammaVec2F other;

  if (gammaCheckVec2F(vm, 1, &other)) {
    gammaError(vm, "Vec2F parameter expected for `other`.");
    return;
  }

  agateSlotSetBool(vm, AGATE_RETURN_SLOT, vec->v[0] != other.v[0] || vec->v[1] != other.v[1]);
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
  vec->v[0] = 0;
  vec->v[1] = 0;
}

static void gammaVec2INew(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_VEC2I_TAG);
  struct GammaVec2I *vec = agateSlotGetForeign(vm, 0);

  if (!gammaCheckInt(vm, 1, &vec->v[0])) {
    gammaError(vm, "Int parameter expected for `x`.");
    return;
  }

  if (!gammaCheckInt(vm, 2, &vec->v[1])) {
    gammaError(vm, "Int parameter expected for `y`.");
    return;
  }
}

static void gammaVec2IGetX(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_VEC2I_TAG);
  struct GammaVec2I *vec = agateSlotGetForeign(vm, 0);
  agateSlotSetInt(vm, AGATE_RETURN_SLOT, vec->v[0]);
}

static void gammaVec2IGetY(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_VEC2I_TAG);
  struct GammaVec2I *vec = agateSlotGetForeign(vm, 0);
  agateSlotSetInt(vm, AGATE_RETURN_SLOT, vec->v[1]);
}

static void gammaVec2ISetX(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_VEC2I_TAG);
  struct GammaVec2I *vec = agateSlotGetForeign(vm, 0);

  if (!gammaCheckInt(vm, 1, &vec->v[0])) {
    gammaError(vm, "Int parameter expected for `value`.");
  }
}

static void gammaVec2ISetY(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_VEC2I_TAG);
  struct GammaVec2I *vec = agateSlotGetForeign(vm, 0);

  if (!gammaCheckInt(vm, 1, &vec->v[1])) {
    gammaError(vm, "Int parameter expected for `value`.");
  }
}

static void gammaVec2IPlus(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_VEC2I_TAG);
  struct GammaVec2I *vec = agateSlotGetForeign(vm, 0);

  struct GammaVec2I *result = gammaForeignAllocate(vm, AGATE_RETURN_SLOT, "gamma/math", "Vec2I");
  result->v[0] = vec->v[0];
  result->v[1] = vec->v[1];
}

static void gammaVec2IMinus(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_VEC2I_TAG);
  struct GammaVec2I *vec = agateSlotGetForeign(vm, 0);

  struct GammaVec2I *result = gammaForeignAllocate(vm, AGATE_RETURN_SLOT, "gamma/math", "Vec2I");
  result->v[0] = - vec->v[0];
  result->v[1] = - vec->v[1];
}

#define GAMMA_VEC2I_OP(name, op)                                \
static void gammaVec2I ## name(AgateVM *vm) {                   \
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_VEC2I_TAG);     \
  struct GammaVec2I *vec = agateSlotGetForeign(vm, 0);          \
  struct GammaVec2I *result = gammaForeignAllocate(vm, AGATE_RETURN_SLOT, "gamma/math", "Vec2I"); \
  int value;                                                    \
  if (gammaCheckInt(vm, 1, &value)) {                           \
    result->v[0] = vec->v[0] op value;                          \
    result->v[1] = vec->v[1] op value;                          \
    return;                                                     \
  }                                                             \
  struct GammaVec2I other;                                      \
  if (gammaCheckVec2I(vm, 1, &other)) {                         \
    result->v[0] = vec->v[0] op other.v[0];                     \
    result->v[1] = vec->v[1] op other.v[1];                     \
  }                                                             \
  gammaError(vm, "Int or Vec2I parameter expected for `other`."); \
}

GAMMA_VEC2I_OP(Add, +)
GAMMA_VEC2I_OP(Sub, -)
GAMMA_VEC2I_OP(Mul, *)
GAMMA_VEC2I_OP(Div, /)

static void gammaVec2IEq(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_VEC2I_TAG);
  struct GammaVec2I *vec = agateSlotGetForeign(vm, 0);
  struct GammaVec2I other;

  if (gammaCheckVec2I(vm, 1, &other)) {
    gammaError(vm, "Vec2I parameter expected for `other`.");
    return;
  }

  agateSlotSetBool(vm, AGATE_RETURN_SLOT, vec->v[0] == other.v[0] && vec->v[1] == other.v[1]);
}

static void gammaVec2INotEq(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_VEC2I_TAG);
  struct GammaVec2I *vec = agateSlotGetForeign(vm, 0);
  struct GammaVec2I other;

  if (gammaCheckVec2I(vm, 1, &other)) {
    gammaError(vm, "Vec2I parameter expected for `other`.");
    return;
  }

  agateSlotSetBool(vm, AGATE_RETURN_SLOT, vec->v[0] != other.v[0] || vec->v[1] != other.v[1]);
}


/*
 * RectF
 */

// raw functions


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

  if (!gammaCheckFloat(vm, 1, &rect->position.v[0])) {
    gammaError(vm, "Float parameter expected for `x`.");
    return;
  }

  if (!gammaCheckFloat(vm, 2, &rect->position.v[1])) {
    gammaError(vm, "Float parameter expected for `y`.");
    return;
  }

  if (!gammaCheckFloat(vm, 3, &rect->size.v[0])) {
    gammaError(vm, "Float parameter expected for `w`.");
    return;
  }

  if (!gammaCheckFloat(vm, 4, &rect->size.v[1])) {
    gammaError(vm, "Float parameter expected for `h`.");
    return;
  }
}

static void gammaRectFNew2(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_RECTF_TAG);
  struct GammaRectF *rect = agateSlotGetForeign(vm, 0);

  if (!gammaCheckVec2F(vm, 1, &rect->position)) {
    gammaError(vm, "Vec2F parameter expected for `position`.");
    return;
  }

  if (!gammaCheckVec2F(vm, 1, &rect->size)) {
    gammaError(vm, "Vec2F parameter expected for `size`.");
    return;
  }
}

static void gammaRectFNew1(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_RECTF_TAG);
  struct GammaRectF *rect = agateSlotGetForeign(vm, 0);

  if (!gammaCheckVec2F(vm, 1, &rect->size)) {
    gammaError(vm, "Vec2F parameter expected for `size`.");
    return;
  }

  rect->position.v[0] = 0.0f;
  rect->position.v[1] = 0.0f;
}

#define GAMMA_RECTF_GET(name, field)                        \
static void gammaRectFGet ## name(AgateVM *vm) {            \
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_RECTF_TAG); \
  struct GammaRectF *rect = agateSlotGetForeign(vm, 0);     \
  agateSlotSetFloat(vm, AGATE_RETURN_SLOT, rect->field);    \
}

GAMMA_RECTF_GET(X, position.v[0])
GAMMA_RECTF_GET(Y, position.v[1])
GAMMA_RECTF_GET(W, size.v[0])
GAMMA_RECTF_GET(H, size.v[1])

#define GAMMA_RECTF_SET(name, field)                          \
static void gammaRectFSet ## name(AgateVM *vm) {              \
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_RECTF_TAG);   \
  struct GammaRectF *rect = agateSlotGetForeign(vm, 0);       \
  if (!gammaCheckFloat(vm, 1, &rect->field)) {                \
    gammaError(vm, "Float parameter expected for `value`.");  \
    return;                                                   \
  }                                                           \
  agateSlotSetFloat(vm, AGATE_RETURN_SLOT, rect->field);      \
}

GAMMA_RECTF_SET(X, position.v[0])
GAMMA_RECTF_SET(Y, position.v[1])
GAMMA_RECTF_SET(W, size.v[0])
GAMMA_RECTF_SET(H, size.v[1])

static void gammaRectFGetPosition(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_RECTF_TAG);
  struct GammaRectF *rect = agateSlotGetForeign(vm, 0);

  struct GammaVec2F *result = gammaForeignAllocate(vm, AGATE_RETURN_SLOT, "gamma/math", "Vec2F");
  *result = rect->position;
}

static void gammaRectFSetPosition(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_RECTF_TAG);
  struct GammaRectF *rect = agateSlotGetForeign(vm, 0);

  if (!gammaCheckVec2F(vm, 1, &rect->position)) {
    gammaError(vm, "Vec2F parameter expected for `position`.");
    return;
  }

  agateSlotCopy(vm, AGATE_RETURN_SLOT, 1);
}

static void gammaRectFGetSize(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_RECTF_TAG);
  struct GammaRectF *rect = agateSlotGetForeign(vm, 0);

  struct GammaVec2F *result = gammaForeignAllocate(vm, AGATE_RETURN_SLOT, "gamma/math", "Vec2F");
  *result = rect->size;
}

static void gammaRectFSetSize(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_RECTF_TAG);
  struct GammaRectF *rect = agateSlotGetForeign(vm, 0);

  if (!gammaCheckVec2F(vm, 1, &rect->size)) {
    gammaError(vm, "Vec2F parameter expected for `size`.");
    return;
  }

  agateSlotCopy(vm, AGATE_RETURN_SLOT, 1);
}

static void gammaRectFContains(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_RECTF_TAG);
  struct GammaRectF *rect = agateSlotGetForeign(vm, 0);

  struct GammaVec2F vec;

  if (gammaCheckVec2F(vm, 1, &vec)) {
    agateSlotSetBool(vm, AGATE_RETURN_SLOT,
        rect->position.v[0] <= vec.v[0] && vec.v[0] < rect->position.v[0] + rect->size.v[0]
     && rect->position.v[1] <= vec.v[1] && vec.v[1] < rect->position.v[1] + rect->size.v[1]
    );
    return;
  }

  struct GammaRectF other;

  if (gammaCheckRectF(vm, 1, &other)) {
    agateSlotSetBool(vm, AGATE_RETURN_SLOT,
        rect->position.v[0] <= other.position.v[0] && other.position.v[0] + other.size.v[0] <= rect->position.v[0] + rect->size.v[0]
     && rect->position.v[1] <= other.position.v[1] && other.position.v[1] + other.size.v[1] <= rect->position.v[1] + rect->size.v[1]
    );
    return;
  }

  agateSlotSetNil(vm, AGATE_RETURN_SLOT);
  gammaError(vm, "Vec2F or RectF parameter expected for `other`.");
}

static void gammaRectFIntersects(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_RECTF_TAG);
  struct GammaRectF *rect = agateSlotGetForeign(vm, 0);

  struct GammaRectF other;

  if (gammaCheckRectF(vm, 1, &other)) {
    agateSlotSetBool(vm, AGATE_RETURN_SLOT,
        rect->position.v[0] < other.position.v[0] + other.size.v[0] && other.position.v[0] < rect->position.v[0] + rect->size.v[0]
     && rect->position.v[1] < other.position.v[1] + other.size.v[1] && other.position.v[1] < rect->position.v[1] + rect->size.v[1]
    );
    return;
  }

  agateSlotSetNil(vm, AGATE_RETURN_SLOT);
  gammaError(vm, "RectF parameter expected for `other`.");
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

  if (!gammaCheckInt(vm, 1, &rect->position.v[0])) {
    gammaError(vm, "Int parameter expected for `x`.");
    return;
  }

  if (!gammaCheckInt(vm, 2, &rect->position.v[1])) {
    gammaError(vm, "Int parameter expected for `y`.");
    return;
  }

  if (!gammaCheckInt(vm, 3, &rect->size.v[0])) {
    gammaError(vm, "Int parameter expected for `w`.");
    return;
  }

  if (!gammaCheckInt(vm, 4, &rect->size.v[1])) {
    gammaError(vm, "Int parameter expected for `h`.");
    return;
  }
}

static void gammaRectINew2(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_RECTI_TAG);
  struct GammaRectI *rect = agateSlotGetForeign(vm, 0);

  if (!gammaCheckVec2I(vm, 1, &rect->position)) {
    gammaError(vm, "Vec2I parameter expected for `position`.");
    return;
  }

  if (!gammaCheckVec2I(vm, 1, &rect->size)) {
    gammaError(vm, "Vec2I parameter expected for `size`.");
    return;
  }
}

static void gammaRectINew1(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_RECTI_TAG);
  struct GammaRectI *rect = agateSlotGetForeign(vm, 0);

  if (!gammaCheckVec2I(vm, 1, &rect->size)) {
    gammaError(vm, "Vec2I parameter expected for `size`.");
    return;
  }

  rect->position.v[0] = 0;
  rect->position.v[1] = 0;
}

#define GAMMA_RECTI_GET(name, field)                        \
static void gammaRectIGet ## name(AgateVM *vm) {            \
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_RECTI_TAG); \
  struct GammaRectI *rect = agateSlotGetForeign(vm, 0);     \
  agateSlotSetInt(vm, AGATE_RETURN_SLOT, rect->field);      \
}

GAMMA_RECTI_GET(X, position.v[0])
GAMMA_RECTI_GET(Y, position.v[1])
GAMMA_RECTI_GET(W, size.v[0])
GAMMA_RECTI_GET(H, size.v[1])

#define GAMMA_RECTI_SET(name, field)                        \
static void gammaRectISet ## name(AgateVM *vm) {            \
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_RECTI_TAG); \
  struct GammaRectI *rect = agateSlotGetForeign(vm, 0);     \
  if (!gammaCheckInt(vm, 1, &rect->field)) {                \
    gammaError(vm, "Int parameter expected for `value`.");  \
    return;                                                 \
  }                                                         \
  agateSlotSetInt(vm, AGATE_RETURN_SLOT, rect->field);      \
}

GAMMA_RECTI_SET(X, position.v[0])
GAMMA_RECTI_SET(Y, position.v[1])
GAMMA_RECTI_SET(W, size.v[0])
GAMMA_RECTI_SET(H, size.v[1])

static void gammaRectIGetPosition(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_RECTI_TAG);
  struct GammaRectI *rect = agateSlotGetForeign(vm, 0);

  struct GammaVec2I *result = gammaForeignAllocate(vm, AGATE_RETURN_SLOT, "gamma/math", "Vec2I");
  *result = rect->position;
}

static void gammaRectISetPosition(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_RECTI_TAG);
  struct GammaRectI *rect = agateSlotGetForeign(vm, 0);

  if (!gammaCheckVec2I(vm, 1, &rect->position)) {
    gammaError(vm, "Vec2I parameter expected for `position`.");
    return;
  }

  agateSlotCopy(vm, AGATE_RETURN_SLOT, 1);
}

static void gammaRectIGetSize(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_RECTI_TAG);
  struct GammaRectI *rect = agateSlotGetForeign(vm, 0);

  struct GammaVec2I *result = gammaForeignAllocate(vm, AGATE_RETURN_SLOT, "gamma/math", "Vec2I");
  *result = rect->size;
}

static void gammaRectISetSize(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_RECTI_TAG);
  struct GammaRectI *rect = agateSlotGetForeign(vm, 0);

  if (!gammaCheckVec2I(vm, 1, &rect->size)) {
    gammaError(vm, "Vec2I parameter expected for `size`.");
    return;
  }

  agateSlotCopy(vm, AGATE_RETURN_SLOT, 1);
}

static void gammaRectIContains(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_RECTI_TAG);
  struct GammaRectI *rect = agateSlotGetForeign(vm, 0);

  struct GammaVec2I vec;

  if (gammaCheckVec2I(vm, 1, &vec)) {
    agateSlotSetBool(vm, AGATE_RETURN_SLOT,
        rect->position.v[0] <= vec.v[0] && vec.v[0] < rect->position.v[0] + rect->size.v[0]
     && rect->position.v[1] <= vec.v[1] && vec.v[1] < rect->position.v[1] + rect->size.v[1]
    );
    return;
  }

  struct GammaRectI other;

  if (gammaCheckRectI(vm, 1, &other)) {
    agateSlotSetBool(vm, AGATE_RETURN_SLOT,
        rect->position.v[0] <= other.position.v[0] && other.position.v[0] + other.size.v[0] <= rect->position.v[0] + rect->size.v[0]
     && rect->position.v[1] <= other.position.v[1] && other.position.v[1] + other.size.v[1] <= rect->position.v[1] + rect->size.v[1]
    );
    return;
  }

  agateSlotSetNil(vm, AGATE_RETURN_SLOT);
  gammaError(vm, "Vec2I or RectI parameter expected for `other`.");
}

static void gammaRectIIntersects(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_RECTI_TAG);
  struct GammaRectI *rect = agateSlotGetForeign(vm, 0);

  struct GammaRectI other;

  if (gammaCheckRectI(vm, 1, &other)) {
    agateSlotSetBool(vm, AGATE_RETURN_SLOT,
        rect->position.v[0] < other.position.v[0] + other.size.v[0] && other.position.v[0] < rect->position.v[0] + rect->size.v[0]
     && rect->position.v[1] < other.position.v[1] + other.size.v[1] && other.position.v[1] < rect->position.v[1] + rect->size.v[1]
    );
    return;
  }

  agateSlotSetNil(vm, AGATE_RETURN_SLOT);
  gammaError(vm, "RectI parameter expected for `other`.");
}

/*
 * Mat3F
 */

// raw functions

void gammaMat3FRawTranslation(struct GammaMat3F *mat, struct GammaVec2F offset) {
  mat->m[0][0] = 1.0; mat->m[1][0] = 0.0; mat->m[2][0] = offset.v[0];
  mat->m[0][1] = 0.0; mat->m[1][1] = 1.0; mat->m[2][1] = offset.v[1];
  mat->m[0][2] = 0.0; mat->m[1][2] = 0.0; mat->m[2][2] = 1.0;
}

void gammaMat3FRawMul(struct GammaMat3F *result, struct GammaMat3F *lhs, struct GammaMat3F *rhs) {
  for (int col = 0; col < 3; ++col) {
    for (int row = 0; row < 3; ++row) {
      float sum = 0.0f;

      for (int k = 0; k < 3; ++k) {
        sum += lhs->m[k][row] * rhs->m[col][k];
      }

      result->m[col][row] = sum;
    }
  }
}

void gammaMat3FRawTransformPoint(struct GammaVec2F *result, struct GammaMat3F *lhs, struct GammaVec2F *rhs) {
  result->v[0] = lhs->m[0][0] * rhs->v[0] + lhs->m[1][0] * rhs->v[1] +  lhs->m[2][0];
  result->v[1] = lhs->m[0][1] * rhs->v[1] + lhs->m[1][1] * rhs->v[1] +  lhs->m[2][1];
}

bool gammaMat3FRawInverse(struct GammaMat3F *result, const struct GammaMat3F *mat) {
  result->m[0][0] = mat->m[1][1] * mat->m[2][2] - mat->m[1][2] * mat->m[2][1];
  result->m[1][0] = - (mat->m[1][0] * mat->m[2][2] - mat->m[1][2] * mat->m[2][0]);
  result->m[2][0] = mat->m[1][0] * mat->m[2][1] - mat->m[1][1] * mat->m[2][0];
  result->m[0][1] = - (mat->m[0][1] * mat->m[2][2] - mat->m[0][2] * mat->m[2][1]);
  result->m[1][1] = mat->m[0][0] * mat->m[2][2] - mat->m[0][2] * mat->m[2][0];
  result->m[2][1] = - (mat->m[0][0] * mat->m[2][1] - mat->m[0][1] * mat->m[2][0]);
  result->m[0][2] = mat->m[0][1] * mat->m[1][2] - mat->m[0][2] * mat->m[1][1];
  result->m[1][2] = - (mat->m[0][0] * mat->m[1][2] - mat->m[0][2] * mat->m[1][0]);
  result->m[2][2] = mat->m[0][0] * mat->m[1][1] - mat->m[0][1] * mat->m[1][0];

  float det = mat->m[0][0] * result->m[0][0] + mat->m[1][0] * result->m[0][1] + mat->m[2][0] * result->m[0][2];

  if (fabsf(det) < FLT_EPSILON) {
    return false;
  }

  for (int col = 0; col < 3; ++col) {
    for (int row = 0; row < 3; ++row) {
      result->m[col][row] /= det;
    }
  }

  return true;
}

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

  mat->m[0][0] = 1.0; mat->m[1][0] = 0.0; mat->m[2][0] = 0.0;
  mat->m[0][1] = 0.0; mat->m[1][1] = 1.0; mat->m[2][1] = 0.0;
  mat->m[0][2] = 0.0; mat->m[1][2] = 0.0; mat->m[2][2] = 1.0;
}

static void gammaMat3FTranslation(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_MAT3F_TAG);
  struct GammaMat3F *mat = agateSlotGetForeign(vm, 0);

  float tx;

  if (!gammaCheckFloat(vm, 1, &tx)) {
    gammaError(vm, "Float parameter expected for `tx`.");
    return;
  }

  float ty;

  if (!gammaCheckFloat(vm, 2, &ty)) {
    gammaError(vm, "Float parameter expected for `ty`.");
    return;
  }

  mat->m[0][0] = 1.0; mat->m[1][0] = 0.0; mat->m[2][0] = tx;
  mat->m[0][1] = 0.0; mat->m[1][1] = 1.0; mat->m[2][1] = ty;
  mat->m[0][2] = 0.0; mat->m[1][2] = 0.0; mat->m[2][2] = 1.0;
}

static void gammaMat3FRotation1(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_MAT3F_TAG);
  struct GammaMat3F *mat = agateSlotGetForeign(vm, 0);

  float angle;

  if (!gammaCheckFloat(vm, 1, &angle)) {
    gammaError(vm, "Float parameter expected for `angle`.");
    return;
  }

  float c = cosf(angle);
  float s = sinf(angle);

  mat->m[0][0] =  c ; mat->m[1][0] = -s ; mat->m[2][0] = 0.0;
  mat->m[0][1] =  s ; mat->m[1][1] =  c ; mat->m[2][1] = 0.0;
  mat->m[0][2] = 0.0; mat->m[1][2] = 0.0; mat->m[2][2] = 1.0;
}

static void gammaMat3FRotation3(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_MAT3F_TAG);
  struct GammaMat3F *mat = agateSlotGetForeign(vm, 0);

  float angle;

  if (!gammaCheckFloat(vm, 1, &angle)) {
    gammaError(vm, "Float parameter expected for `angle`.");
    return;
  }

  float cx;

  if (!gammaCheckFloat(vm, 2, &cx)) {
    gammaError(vm, "Float parameter expected for `cx`.");
    return;
  }

  float cy;

  if (!gammaCheckFloat(vm, 3, &cy)) {
    gammaError(vm, "Float parameter expected for `cy`.");
    return;
  }

  float cos_v = cosf(angle);
  float sin_v = sinf(angle);

  mat->m[0][0] = cos_v; mat->m[1][0] = -sin_v; mat->m[2][0] = cx * (1 - cos_v) + cy * sin_v;
  mat->m[0][1] = sin_v; mat->m[1][1] =  cos_v; mat->m[2][1] = cy * (1 - cos_v) - cy * sin_v;
  mat->m[0][2] =   0.0; mat->m[1][2] =    0.0; mat->m[2][2] = 1.0;
}

static void gammaMat3FScale(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_MAT3F_TAG);
  struct GammaMat3F *mat = agateSlotGetForeign(vm, 0);

  float sx;

  if (!gammaCheckFloat(vm, 1, &sx)) {
    gammaError(vm, "Float parameter expected for `sx`.");
    return;
  }

  float sy;

  if (!gammaCheckFloat(vm, 2, &sy)) {
    gammaError(vm, "Float parameter expected for `sy`.");
    return;
  }

  mat->m[0][0] =  sx; mat->m[1][0] = 0.0; mat->m[2][0] = 0.0;
  mat->m[0][1] = 0.0; mat->m[1][1] =  sy; mat->m[2][1] = 0.0;
  mat->m[0][2] = 0.0; mat->m[1][2] = 0.0; mat->m[2][2] = 1.0;
}

static void gammaMat3FSubscript(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_MAT3F_TAG);
  struct GammaMat3F *mat = agateSlotGetForeign(vm, 0);

  int row;

  if (!gammaCheckInt(vm, 1, &row)) {
    gammaError(vm, "Int parameter expected for `row`.");
    return;
  }

  if (row < 0 || row >= 3) {
    gammaError(vm, "Out of bounds: `row`.");
    return;
  }

  int col;

  if (!gammaCheckInt(vm, 2, &col)) {
    gammaError(vm, "Int parameter expected for `col`.");
    return;
  }

  if (col < 0 || col >= 3) {
    gammaError(vm, "Out of bounds: `col`.");
    return;
  }

  agateSlotSetFloat(vm, AGATE_RETURN_SLOT, mat->m[col][row]);
}

static void gammaMat3FAdd(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_MAT3F_TAG);
  struct GammaMat3F *mat = agateSlotGetForeign(vm, 0);

  assert(agateSlotGetForeignTag(vm, 1) == GAMMA_MAT3F_TAG);
  struct GammaMat3F *other = agateSlotGetForeign(vm, 1);

  struct GammaMat3F *result = gammaForeignAllocate(vm, AGATE_RETURN_SLOT, "gamma/math", "Mat3F");

  for (int col = 0; col < 3; ++col) {
    for (int row = 0; row < 3; ++row) {
      result->m[col][row] = mat->m[col][row] + other->m[col][row];
    }
  }
}

static void gammaMat3FSub(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_MAT3F_TAG);
  struct GammaMat3F *mat = agateSlotGetForeign(vm, 0);

  assert(agateSlotGetForeignTag(vm, 1) == GAMMA_MAT3F_TAG);
  struct GammaMat3F *other = agateSlotGetForeign(vm, 1);

  struct GammaMat3F *result = gammaForeignAllocate(vm, AGATE_RETURN_SLOT, "gamma/math", "Mat3F");

  for (int col = 0; col < 3; ++col) {
    for (int row = 0; row < 3; ++row) {
      result->m[col][row] = mat->m[col][row] - other->m[col][row];
    }
  }
}

static void gammaMat3FMul(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_MAT3F_TAG);
  struct GammaMat3F *mat = agateSlotGetForeign(vm, 0);

  /* if (agateSlotGetForeignTag(vm, 1) == GAMMA_MAT3F_TAG) {
    struct GammaVec3F *vec = agateSlotGetForeign(vm, 1);

    struct GammaVec3F *result = gammaForeignAllocate(vm, AGATE_RETURN_SLOT, "gamma/math", "Mat3F");

    result->position.v[0] = mat->m[0][0] * vec->position.v[0] + mat->m[1][0] * vec->position.v[1] + mat->m[2][0] * vec->z;
    result->position.v[1] = mat->m[0][1] * vec->position.v[0] + mat->m[1][1] * vec->position.v[1] + mat->m[2][1] * vec->z;
    result->z = mat->m[0][2] * vec->position.v[0] + mat->m[1][2] * vec->position.v[1] + mat->m[2][2] * vec->z;
  } else */
  if (agateSlotGetForeignTag(vm, 1) == GAMMA_MAT3F_TAG) {
    struct GammaMat3F *other = agateSlotGetForeign(vm, 1);
    struct GammaMat3F *result = gammaForeignAllocate(vm, AGATE_RETURN_SLOT, "gamma/math", "Mat3F");

    gammaMat3FRawMul(result, mat, other);
  } else {
    // TODO: error
  }
}

static void gammaMat3FTranspose(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_MAT3F_TAG);
  struct GammaMat3F *mat = agateSlotGetForeign(vm, 0);

  struct GammaMat3F *result = gammaForeignAllocate(vm, AGATE_RETURN_SLOT, "gamma/math", "Mat3F");

  for (int col = 0; col < 3; ++col) {
    for (int row = 0; row < 3; ++row) {
      result->m[col][row] = mat->m[row][col];
    }
  }
}

static void gammaMat3FInvert(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_MAT3F_TAG);
  struct GammaMat3F *mat = agateSlotGetForeign(vm, 0);

  struct GammaMat3F *result = gammaForeignAllocate(vm, AGATE_RETURN_SLOT, "gamma/math", "Mat3F");

  result->m[0][0] = mat->m[1][1] * mat->m[2][2] - mat->m[2][1] * mat->m[1][2];
  result->m[0][1] = - (mat->m[0][1] * mat->m[2][2] - mat->m[2][1] * mat->m[0][2]);
  result->m[0][2] = mat->m[0][1] * mat->m[1][2] - mat->m[1][1] * mat->m[0][2];
  result->m[1][0] = - (mat->m[1][0] * mat->m[2][2] - mat->m[2][0] * mat->m[1][2]);
  result->m[1][1] = mat->m[0][0] * mat->m[2][2] - mat->m[2][0] * mat->m[0][2];
  result->m[1][2] = - (mat->m[0][0] * mat->m[1][2] - mat->m[1][0] * mat->m[0][2]);
  result->m[2][0] = mat->m[1][0] * mat->m[2][1] - mat->m[2][0] * mat->m[1][1];
  result->m[2][1] = - (mat->m[0][0] * mat->m[2][1] - mat->m[2][0] * mat->m[0][1]);
  result->m[2][2] = mat->m[0][0] * mat->m[1][1] - mat->m[1][0] * mat->m[0][1];

  float det = mat->m[0][0] * result->m[0][0] + mat->m[0][1] * result->m[1][0] + mat->m[0][2] * result->m[2][0];

  if (det < FLT_EPSILON) {
    agateSlotSetNil(vm, AGATE_RETURN_SLOT);
    return;
  }

  for (int col = 0; col < 3; ++col) {
    for (int row = 0; row < 3; ++row) {
      result->m[col][row] /= det;
    }
  }
}

static void gammaMat3FTransformPoint(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_MAT3F_TAG);
  struct GammaMat3F *mat = agateSlotGetForeign(vm, 0);

  float x = (float) agateSlotGetFloat(vm, 1);
  float y = (float) agateSlotGetFloat(vm, 2);
  // z == 1.0f

  struct GammaVec2F *result = gammaForeignAllocate(vm, AGATE_RETURN_SLOT, "gamma/math", "Vec2F");

  result->v[0] = mat->m[0][0] * x + mat->m[1][0] * y +  mat->m[2][0];
  result->v[1] = mat->m[0][1] * x + mat->m[1][1] * y +  mat->m[2][1];
}

static void gammaMat3FTransformVector(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_MAT3F_TAG);
  struct GammaMat3F *mat = agateSlotGetForeign(vm, 0);

  float x = (float) agateSlotGetFloat(vm, 1);
  float y = (float) agateSlotGetFloat(vm, 2);
  // z == 0.0f

  struct GammaVec2F *result = gammaForeignAllocate(vm, AGATE_RETURN_SLOT, "gamma/math", "Vec2F");

  result->v[0] = mat->m[0][0] * x + mat->m[1][0] * y;
  result->v[1] = mat->m[0][1] * x + mat->m[1][1] * y;
}

/*
 * unit configuration
 */

AgateForeignClassHandler gammaMathClassHandler(AgateVM *vm, const char *unit_name, const char *class_name) {
  assert(gammaEquals(unit_name, "gamma/math"));
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

AgateForeignMethodFunc gammaMathMethodHandler(AgateVM *vm, const char *unit_name, const char *class_name, AgateForeignMethodKind kind, const char *signature) {
  assert(gammaEquals(unit_name, "gamma/math"));

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
      if (gammaEquals(signature, "position")) { return gammaRectFGetPosition; }
      if (gammaEquals(signature, "size")) { return gammaRectFGetSize; }
      if (gammaEquals(signature, "position=(_)")) { return gammaRectFSetPosition; }
      if (gammaEquals(signature, "size=(_)")) { return gammaRectFSetSize; }
      if (gammaEquals(signature, "contains(_)")) { return gammaRectFContains; }
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
      if (gammaEquals(signature, "position")) { return gammaRectIGetPosition; }
      if (gammaEquals(signature, "size")) { return gammaRectIGetSize; }
      if (gammaEquals(signature, "position=(_)")) { return gammaRectISetPosition; }
      if (gammaEquals(signature, "size=(_)")) { return gammaRectISetSize; }
      if (gammaEquals(signature, "contains(_)")) { return gammaRectIContains; }
      if (gammaEquals(signature, "intersects(_)")) { return gammaRectIIntersects; }
    }
  }

  if (gammaEquals(class_name, "Mat3F")) {
    if (kind == AGATE_FOREIGN_METHOD_INSTANCE) {
      if (gammaEquals(signature, "init identity()")) { return gammaMat3FIdentity; }
      if (gammaEquals(signature, "init translation(_,_)")) { return gammaMat3FTranslation; }
      if (gammaEquals(signature, "init rotation(_)")) { return gammaMat3FRotation1; }
      if (gammaEquals(signature, "init rotation(_,_,_)")) { return gammaMat3FRotation3; }
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
