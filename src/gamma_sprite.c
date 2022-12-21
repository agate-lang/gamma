#include "gamma_sprite.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "gamma_check.h"
#include "gamma_common.h"
#include "gamma_debug.h"
#include "gamma_error.h"
#include "gamma_math.h"
#include "gamma_render.h"
#include "gamma_tags.h"
#include "gamma_utils.h"

/*
 * Image
 */

struct GammaImage {
  int width;
  int height;
  uint8_t *pixels;
};

// raw functions

static void gammaImageRawFlip(struct GammaImage *image) {
  if (image->width == 0 || image->height == 0 || image->pixels == NULL) {
    return;
  }

  int stride = image->width * 4;
  uint8_t *upper = image->pixels;
  uint8_t *lower = upper + (image->height - 1) * stride;

  for (int i = 0; i < image->height / 2; ++i) {
    for (int j = 0; j < stride; ++j) {
      uint8_t tmp = upper[j];
      upper[j] = lower[j];
      lower[j] = tmp;
    }

    upper += stride;
    lower -= stride;
  }
}

static bool gammaImageRawLoad(struct GammaImage *image, const char *filename) {
  int n = 0;
  image->pixels = stbi_load(filename, &image->width, &image->height, &n, STBI_rgb_alpha);

  if (image->width == 0 || image->height == 0 || image->pixels == NULL) {
    return false;
  }

  gammaImageRawFlip(image);
  return true;
}

static void gammaImageRawFree(struct GammaImage *image) {
  free(image->pixels);
  image->pixels = NULL;
  image->width = image->height = 0;
}

static void gammaImageRawGet(struct GammaImage *image, int x, int y, struct GammaColor *color) {
  if (x < 0 || x >= image->width || y < 0 || y > image->height) {
    color->r = color->g = color->b = color->a = 0.0f;
    return;
  }

  uint8_t *ptr = image->pixels + (x + (image->height - y - 1) * image->width) * 4;
  color->r = ptr[0] / 255.0f;
  color->g = ptr[1] / 255.0f;
  color->b = ptr[2] / 255.0f;
  color->a = ptr[3] / 255.0f;
}

static void gammaImageRawSet(struct GammaImage *image, int x, int y, struct GammaColor *color) {
  if (x < 0 || x >= image->width || y < 0 || y > image->height) {
    return;
  }

  uint8_t *ptr = image->pixels + (x + (image->height - y - 1) * image->width) * 4;
  ptr[0] = (uint8_t) (255 * gammaClampF(color->r, 0.0f, 1.0f));
  ptr[1] = (uint8_t) (255 * gammaClampF(color->g, 0.0f, 1.0f));
  ptr[2] = (uint8_t) (255 * gammaClampF(color->b, 0.0f, 1.0f));
  ptr[3] = (uint8_t) (255 * gammaClampF(color->a, 0.0f, 1.0f));
}

// class

static ptrdiff_t gammaImageAllocate(AgateVM *vm, const char *unit_name, const char *class_name) {
  return sizeof(struct GammaImage);
}

static uint64_t gammaImageTag(AgateVM *vm, const char *unit_name, const char *class_name) {
  return GAMMA_IMAGE_TAG;
}

static void gammaImageDestroy(AgateVM *vm, const char *unit_name, const char *class_name, void *data) {
  struct GammaImage *image = data;
  gammaImageRawFree(image);
}

// methods

static void gammaImageFromFile(AgateVM *vm) {
  assert(gammaCheckForeign(vm, 0, GAMMA_IMAGE_TAG));
  struct GammaImage *image = agateSlotGetForeign(vm, 0);

  const char *filename = NULL;

  if (!gammaCheckString(vm, 1, &filename)) {
    gammaError(vm, "String parameter expected for `filename`.");
    return;
  }

  if (!gammaImageRawLoad(image, filename)) {
    gammaError(vm, "Unable to load image: '%s'.", filename);
    return;
  }
}

static void gammaImageSubscriptGetter(AgateVM *vm) {
  assert(gammaCheckForeign(vm, 0, GAMMA_IMAGE_TAG));
  struct GammaImage *image = agateSlotGetForeign(vm, 0);

  int x;

  if (!gammaCheckInt(vm, 1, &x)) {
    gammaError(vm, "Int parameter expected for `x`.");
    return;
  }

  int y;

  if (!gammaCheckInt(vm, 2, &y)) {
    gammaError(vm, "Int parameter expected for `y`.");
    return;
  }

  struct GammaColor *result = gammaForeignAllocate(vm, AGATE_RETURN_SLOT, "gamma/math", "Color");
  gammaImageRawGet(image, x, y, result);
}

static void gammaImageSubscriptSetter(AgateVM *vm) {
  assert(gammaCheckForeign(vm, 0, GAMMA_IMAGE_TAG));
  struct GammaImage *image = agateSlotGetForeign(vm, 0);

  int x;

  if (!gammaCheckInt(vm, 1, &x)) {
    gammaError(vm, "Int parameter expected for `x`.");
    return;
  }

  int y;

  if (!gammaCheckInt(vm, 2, &y)) {
    gammaError(vm, "Int parameter expected for `y`.");
    return;
  }

  struct GammaColor color;

  if (!gammaCheckColor(vm, 3, &color)) {
    gammaError(vm, "Color parameter expected for `color`.");
    return;
  }

  gammaImageRawSet(image, x, y, &color);
}

/*
 * Texture
 */

#define GAMMA_TEXTURE_SMOOTH    0x01
#define GAMMA_TEXTURE_REPEATED  0x02
#define GAMMA_TEXTURE_MIPMAP    0x04

struct GammaTexture {
  enum GammaTextureKind kind;
  GLuint id;
  GLsizei width;
  GLsizei height;
  uint32_t flags;
};

// raw functions

GLuint gammaTextureRawCreate(GLsizei width, GLsizei height, enum GammaTextureKind kind, const uint8_t *data) {
  GLuint texture = 0;
  GAMMA_GL_CHECK(glGenTextures(1, &texture));

  if (texture == 0) {
    return 0;
  }

  GLint alignment = (kind == GAMMA_TEXTURE_COLOR) ? 4 : 1;
  GLenum format = (kind == GAMMA_TEXTURE_COLOR) ? GL_RGBA : GL_RED;

  GAMMA_GL_CHECK(glPixelStorei(GL_UNPACK_ALIGNMENT, alignment));
  GAMMA_GL_CHECK(glBindTexture(GL_TEXTURE_2D, texture));
  GAMMA_GL_CHECK(glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data));
  GAMMA_GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
  GAMMA_GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
  GAMMA_GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
  GAMMA_GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
  GAMMA_GL_CHECK(glBindTexture(GL_TEXTURE_2D, 0));
  return texture;
}

static inline bool gammaTextureRawIsSmooth(const struct GammaTexture *texture) {
  return (texture->flags & GAMMA_TEXTURE_SMOOTH) != 0;
}

static inline bool gammaTextureRawIsRepeated(const struct GammaTexture *texture) {
  return (texture->flags & GAMMA_TEXTURE_REPEATED) != 0;
}

static inline bool gammaTextureRawHasMipmap(const struct GammaTexture *texture) {
  return (texture->flags & GAMMA_TEXTURE_MIPMAP) != 0;
}

static GLenum gammaTextureRawMinFilter(const struct GammaTexture *texture) {
  if (gammaTextureRawHasMipmap(texture)) {
    return gammaTextureRawIsSmooth(texture) ? GL_LINEAR_MIPMAP_LINEAR : GL_NEAREST_MIPMAP_LINEAR;
  }

  return gammaTextureRawIsSmooth(texture) ? GL_LINEAR : GL_NEAREST;
}


// class

static ptrdiff_t gammaTextureAllocate(AgateVM *vm, const char *unit_name, const char *class_name) {
  return sizeof(struct GammaTexture);
}

static uint64_t gammaTextureTag(AgateVM *vm, const char *unit_name, const char *class_name) {
  return GAMMA_TEXTURE_TAG;
}

static void gammaTextureDestroy(AgateVM *vm, const char *unit_name, const char *class_name, void *data) {
  struct GammaTexture *texture = data;

  if (texture->id != 0) {
    glDeleteTextures(1, &texture->id);
    texture->id = 0;
  }

  texture->width = 0;
  texture->height = 0;
  texture->flags = 0;
}

// methods

static void gammaTextureFromFile(AgateVM *vm) {
  assert(gammaCheckForeign(vm, 0, GAMMA_TEXTURE_TAG));
  struct GammaTexture *texture = agateSlotGetForeign(vm, 0);

  const char *filename = NULL;

  if (!gammaCheckString(vm, 1, &filename)) {
    gammaError(vm, "String parameter expected for `filename`.");
    return;
  }

  struct GammaImage image;

  if (!gammaImageRawLoad(&image, filename)) {
    gammaError(vm, "Unable to load texture: '%s'.", filename);
    return;
  }

  texture->kind = GAMMA_TEXTURE_COLOR;
  texture->width = image.width;
  texture->height = image.height;
  texture->id = gammaTextureRawCreate(texture->width, texture->height, texture->kind, image.pixels);
  texture->flags = 0;

  gammaImageRawFree(&image);
}

static void gammaTextureFromImage(AgateVM *vm) {
  assert(gammaCheckForeign(vm, 0, GAMMA_TEXTURE_TAG));
  struct GammaTexture *texture = agateSlotGetForeign(vm, 0);

  if (!gammaCheckForeign(vm, 1, GAMMA_IMAGE_TAG)) {
    gammaError(vm, "Image parameter expected for `image`.");
    return;
  }

  struct GammaImage *image = agateSlotGetForeign(vm, 1);

  texture->kind = GAMMA_TEXTURE_COLOR;
  texture->width = image->width;
  texture->height = image->height;
  texture->id = gammaTextureRawCreate(texture->width, texture->height, texture->kind, image->pixels);
  texture->flags = GAMMA_TEXTURE_SMOOTH & GAMMA_TEXTURE_REPEATED;
}

static void gammaTextureGetSize(AgateVM *vm) {
  assert(gammaCheckForeign(vm, 0, GAMMA_TEXTURE_TAG));
  struct GammaTexture *texture = agateSlotGetForeign(vm, 0);

  struct GammaVec2I *result = gammaForeignAllocate(vm, AGATE_RETURN_SLOT, "gamma/math", "Vec2I");
  result->v[0] = texture->width;
  result->v[1] = texture->height;
}

static void gammaTextureSetSize(AgateVM *vm) {
  assert(gammaCheckForeign(vm, 0, GAMMA_TEXTURE_TAG));
  struct GammaTexture *texture = agateSlotGetForeign(vm, 0);

  struct GammaVec2I size;

  if (!gammaCheckVec2I(vm, 1, &size)) {
    gammaError(vm, "Vec2I parameter expected for `value`.");
    return;
  }

  texture->width = size.v[0];
  texture->height = size.v[1];

  assert(texture->kind == GAMMA_TEXTURE_COLOR);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
  glBindTexture(GL_TEXTURE_2D, texture->id);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture->width, texture->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
  glBindTexture(GL_TEXTURE_2D, 0);
}

static void gammaTextureIsSmooth(AgateVM *vm) {
  assert(gammaCheckForeign(vm, 0, GAMMA_TEXTURE_TAG));
  struct GammaTexture *texture = agateSlotGetForeign(vm, 0);

  agateSlotSetBool(vm, AGATE_RETURN_SLOT, gammaTextureRawIsSmooth(texture));
}

static void gammaTextureSetSmooth(AgateVM *vm) {
  assert(gammaCheckForeign(vm, 0, GAMMA_TEXTURE_TAG));
  struct GammaTexture *texture = agateSlotGetForeign(vm, 0);

  bool smooth;

  if (!gammaCheckBool(vm, 1, &smooth)) {
    gammaError(vm, "Bool parameter expected for `value`.");
    return;
  }

  if (smooth == gammaTextureRawIsSmooth(texture)) {
    return;
  }

  if (smooth) {
    texture->flags |= GAMMA_TEXTURE_SMOOTH;
  } else {
    texture->flags &= ~GAMMA_TEXTURE_SMOOTH;
  }

  glBindTexture(GL_TEXTURE_2D, texture->id);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, smooth ? GL_LINEAR : GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gammaTextureRawMinFilter(texture));
  glBindTexture(GL_TEXTURE_2D, 0);
}

static void gammaTextureIsRepeated(AgateVM *vm) {
  assert(gammaCheckForeign(vm, 0, GAMMA_TEXTURE_TAG));
  struct GammaTexture *texture = agateSlotGetForeign(vm, 0);

  agateSlotSetBool(vm, AGATE_RETURN_SLOT, gammaTextureRawIsRepeated(texture));
}

static void gammaTextureSetRepeated(AgateVM *vm) {
  assert(gammaCheckForeign(vm, 0, GAMMA_TEXTURE_TAG));
  struct GammaTexture *texture = agateSlotGetForeign(vm, 0);

  bool repeated;

  if (!gammaCheckBool(vm, 1, &repeated)) {
    gammaError(vm, "Bool parameter expected for `value`.");
    return;
  }

  if (repeated == gammaTextureRawIsRepeated(texture)) {
    return;
  }

  if (repeated) {
    texture->flags |= GAMMA_TEXTURE_REPEATED;
  } else {
    texture->flags &= ~GAMMA_TEXTURE_REPEATED;
  }

  glBindTexture(GL_TEXTURE_2D, texture->id);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, repeated ? GL_REPEAT : GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, repeated ? GL_REPEAT : GL_CLAMP_TO_EDGE);
  glBindTexture(GL_TEXTURE_2D, 0);
}

static void gammaTextureGenerateMipmap(AgateVM *vm) {
  assert(gammaCheckForeign(vm, 0, GAMMA_TEXTURE_TAG));
  struct GammaTexture *texture = agateSlotGetForeign(vm, 0);

  if (gammaTextureRawHasMipmap(texture)) {
    return;
  }

  texture->flags |= GAMMA_TEXTURE_MIPMAP;

  glBindTexture(GL_TEXTURE_2D, texture->id);
  glGenerateMipmap(GL_TEXTURE_2D);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gammaTextureRawMinFilter(texture));
  glBindTexture(GL_TEXTURE_2D, 0);

}

/*
 * Sprite
 */

struct GammaSprite {
  GLuint buffer;
  struct GammaColor color;
  struct {
    GLuint id;
    struct GammaVec2I size;
    struct GammaRectF region;
    AgateHandle *handle;
  } texture;
};

void gammaSpriteUpdateBuffer(const struct GammaSprite *sprite) {
  assert(sprite->buffer != 0);

  struct GammaRectF bounds = { {{ 0.0f, 0.0f }}, sprite->texture.region.size };
  bounds.size.v[0] *= sprite->texture.size.v[0];
  bounds.size.v[1] *= sprite->texture.size.v[1];

  struct GammaVertex vertices[] = {
    { gammaRectFRawPosition(&bounds, 0.0f, 0.0f, false), sprite->color, gammaRectFRawPosition(&sprite->texture.region, 0.0f, 0.0f, true) },
    { gammaRectFRawPosition(&bounds, 0.0f, 1.0f, false), sprite->color, gammaRectFRawPosition(&sprite->texture.region, 0.0f, 1.0f, true) },
    { gammaRectFRawPosition(&bounds, 1.0f, 0.0f, false), sprite->color, gammaRectFRawPosition(&sprite->texture.region, 1.0f, 0.0f, true) },
    { gammaRectFRawPosition(&bounds, 1.0f, 1.0f, false), sprite->color, gammaRectFRawPosition(&sprite->texture.region, 1.0f, 1.0f, true) },
  };

  glBindBuffer(GL_ARRAY_BUFFER, sprite->buffer);
  glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void gammaSpriteRawRender(AgateVM *vm, ptrdiff_t slot, struct GammaRendererData *data) {
  assert(gammaCheckForeign(vm, slot, GAMMA_SPRITE_TAG));
  struct GammaSprite *sprite = agateSlotGetForeign(vm, slot);

  data->primitive = GL_TRIANGLE_STRIP;
  data->count = 4;
  data->vertex_buffer = sprite->buffer;
  data->element_buffer = 0;
  data->mode = GAMMA_RENDERER_COLOR;
  data->texture0 = sprite->texture.id;
  data->texture1 = 0;
  data->shader = 0;

  struct GammaRectF bounds = { {{ 0.0f, 0.0f }}, sprite->texture.region.size };
  bounds.size.v[0] *= sprite->texture.size.v[0];
  bounds.size.v[1] *= sprite->texture.size.v[1];

  data->bounds = bounds;
}

// class

static ptrdiff_t gammaSpriteAllocate(AgateVM *vm, const char *unit_name, const char *class_name) {
  return sizeof(struct GammaSprite);
}

static uint64_t gammaSpriteTag(AgateVM *vm, const char *unit_name, const char *class_name) {
  return GAMMA_SPRITE_TAG;
}

static void gammaSpriteDestroy(AgateVM *vm, const char *unit_name, const char *class_name, void *data) {
  struct GammaSprite *sprite = data;

  if (sprite->buffer != 0) {
    glDeleteBuffers(1, &sprite->buffer);
    sprite->buffer = 0;
  }

  if (sprite->texture.handle != NULL) {
    agateReleaseHandle(vm, sprite->texture.handle);
    sprite->texture.handle = NULL;
  }
}

// method

static const struct GammaRectF gammaDefaultTextureRegion = { {{ 0.0f, 0.0f }},  {{ 1.0f, 1.0f }} };
static const struct GammaColor gammaDefaultColor = { 1.0f, 1.0f, 1.0f, 1.0f };

static void gammaSpriteNew(AgateVM *vm) {
  assert(gammaCheckForeign(vm, 0, GAMMA_SPRITE_TAG));
  struct GammaSprite *sprite = agateSlotGetForeign(vm, 0);

  if (!gammaCheckForeign(vm, 1, GAMMA_TEXTURE_TAG)) {
    gammaError(vm, "Texture parameter expected for `texture`.");
    return;
  }

  struct GammaTexture *texture = agateSlotGetForeign(vm, 1);

  sprite->color = gammaDefaultColor;

  glGenBuffers(1, &sprite->buffer);
  glBindBuffer(GL_ARRAY_BUFFER, sprite->buffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(struct GammaVertex) * 4, NULL, GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  sprite->texture.id = texture->id;
  sprite->texture.size.v[0] = texture->width;
  sprite->texture.size.v[1] = texture->height;
  sprite->texture.region = gammaDefaultTextureRegion;
  sprite->texture.handle = agateSlotGetHandle(vm, 1);

  gammaSpriteUpdateBuffer(sprite);
}


/*
 * unit configuration
 */

AgateForeignClassHandler gammaSpriteClassHandler(AgateVM *vm, const char *unit_name, const char *class_name) {
  assert(gammaEquals(unit_name, "gamma/sprite"));
  AgateForeignClassHandler handler = { NULL, NULL, NULL };

  if (gammaEquals(class_name, "Image")) {
    handler.allocate = gammaImageAllocate;
    handler.tag = gammaImageTag;
    handler.destroy = gammaImageDestroy;
    return handler;
  }

  if (gammaEquals(class_name, "Texture")) {
    handler.allocate = gammaTextureAllocate;
    handler.tag = gammaTextureTag;
    handler.destroy = gammaTextureDestroy;
    return handler;
  }

  if (gammaEquals(class_name, "Sprite")) {
    handler.allocate = gammaSpriteAllocate;
    handler.tag = gammaSpriteTag;
    handler.destroy = gammaSpriteDestroy;
    return handler;
  }

  return handler;
}

AgateForeignMethodFunc gammaSpriteMethodHandler(AgateVM *vm, const char *unit_name, const char *class_name, AgateForeignMethodKind kind, const char *signature) {
  assert(gammaEquals(unit_name, "gamma/sprite"));

  if (gammaEquals(class_name, "Image")) {
    if (kind == AGATE_FOREIGN_METHOD_INSTANCE) {
      if (gammaEquals(signature, "init from_file(_)")) { return gammaImageFromFile; }
      if (gammaEquals(signature, "[_,_]")) { return gammaImageSubscriptGetter; }
      if (gammaEquals(signature, "[_,_]=(_)")) { return gammaImageSubscriptSetter; }
    }
  }

  if (gammaEquals(class_name, "Texture")) {
    if (kind == AGATE_FOREIGN_METHOD_INSTANCE) {
      if (gammaEquals(signature, "init from_file(_)")) { return gammaTextureFromFile; }
      if (gammaEquals(signature, "init from_image(_)")) { return gammaTextureFromImage; }
      if (gammaEquals(signature, "size")) { return gammaTextureGetSize; }
      if (gammaEquals(signature, "size=(_)")) { return gammaTextureSetSize; }
      if (gammaEquals(signature, "smooth")) { return gammaTextureIsSmooth; }
      if (gammaEquals(signature, "smooth=(_)")) { return gammaTextureSetSmooth; }
      if (gammaEquals(signature, "repeated")) { return gammaTextureIsRepeated; }
      if (gammaEquals(signature, "repeated=(_)")) { return gammaTextureSetRepeated; }
      if (gammaEquals(signature, "generate_mipmap()")) { return gammaTextureGenerateMipmap; }
    }
  }

  if (gammaEquals(class_name, "Sprite")) {
    if (kind == AGATE_FOREIGN_METHOD_INSTANCE) {
      if (gammaEquals(signature, "init new(_)")) { return gammaSpriteNew; }
    }
  }

  return NULL;
}
