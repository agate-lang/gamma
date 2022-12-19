#include "gamma_sprite.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "gamma_check.h"
#include "gamma_common.h"
#include "gamma_error.h"
#include "gamma_root.h"
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
  free(image->pixels);
  image->pixels = NULL;
  image->width = image->height = 0;
}

// methods

static void gammaImageFromFile(AgateVM *vm) {
  assert(gammaCheckForeign(vm, 0, GAMMA_IMAGE_TAG));
  struct GammaImage *image = agateSlotGetForeign(vm, 0);

  const char *filename = agateSlotGetString(vm, 1);

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

  struct GammaColor *result = gammaForeignAllocate(vm, AGATE_RETURN_SLOT, "Color");
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

// raw functions

GLuint gammaTextureRawCreate(GLsizei width, GLsizei height, GLenum format, GLint alignment, const uint8_t *data) {
  GLuint texture = 0;
  glGenTextures(1, &texture);

  if (texture == 0) {
    return 0;
  }

  glPixelStorei(GL_UNPACK_ALIGNMENT, alignment);
  glBindTexture(GL_TEXTURE_2D, texture);
  glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
  glBindTexture(GL_TEXTURE_2D, 0);
  return texture;
}







AgateForeignClassHandler gammaSpriteClassHandler(AgateVM *vm, const char *unit_name, const char *class_name) {
  assert(gammaEquals(unit_name, "gamma/sprite"));
  AgateForeignClassHandler handler = { NULL, NULL, NULL };

  if (gammaEquals(class_name, "Image")) {
    handler.allocate = gammaImageAllocate;
    handler.tag = gammaImageTag;
    handler.destroy = gammaImageDestroy;
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

  return NULL;
}
