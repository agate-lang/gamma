#include "gamma_sprite.h"

#include <algorithm>

#define STBI_WINDOWS_UTF8
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "gamma_agate.h"
#include "gamma_debug.h"
#include "gamma_render.h"

namespace gma {

  /*
   * Image
   */

  Image::Image(const char *filename) {
    stbi_set_flip_vertically_on_load(true);
    [[maybe_unused]] int n = 0;
    pixels = stbi_load(filename, &width, &height, &n, STBI_rgb_alpha);
    assert(n == 4);
  }

  void Image::destroy() {
    stbi_image_free(pixels);
    pixels = nullptr;
    width = height = 0;
  }

  Color Image::get_color(Vec2I position) const {
    Color color;

    if (position.x < 0 || position.x >= width || position.y < 0 || position.y >= height) {
      color.r = color.g = color.b = color.a = 0.0f;
    } else {
      uint8_t *ptr = pixels + (position.x + (height - position.y - 1) * width) * 4;
      color.r = ptr[0] / 255.0f;
      color.g = ptr[1] / 255.0f;
      color.b = ptr[2] / 255.0f;
      color.a = ptr[3] / 255.0f;
    }

    return color;
  }

  void Image::set_color(Vec2I position, Color color) {
    if (position.x < 0 || position.x >= width || position.y < 0 || position.y >= height) {
      return;
    }

    uint8_t *ptr = pixels + (position.x + (height - position.y - 1) * width) * 4;
    ptr[0] = (uint8_t) (255 * std::clamp(color.r, 0.0f, 1.0f));
    ptr[1] = (uint8_t) (255 * std::clamp(color.g, 0.0f, 1.0f));
    ptr[2] = (uint8_t) (255 * std::clamp(color.b, 0.0f, 1.0f));
    ptr[3] = (uint8_t) (255 * std::clamp(color.a, 0.0f, 1.0f));
  }

  struct ImageApi : ImageClass {

    static void destroy(AgateVM *vm, const char *unit_name, const char *class_name, void *data) {
      auto image = static_cast<Image *>(data);
      image->destroy();
    }

    static void from_file(AgateVM *vm) {
      assert(agateCheckTag<ImageClass>(vm, 0));
      auto image = agateSlotGet<ImageClass>(vm, 0);

      const char *filename = nullptr;

      if (!agateCheck(vm, 1, filename)) {
        agateError(vm, "String parameter expected for `filename`.");
        return;
      }

      *image = Image(filename);

      if (!image->loaded()) {
        agateError(vm, "Unable to load image: '%s'.", filename);
        return;
      }
    }

    static void subscript_getter1(AgateVM *vm) {
      assert(agateCheckTag<ImageClass>(vm, 0));
      auto image = agateSlotGet<ImageClass>(vm, 0);

      Vec2I position;

      if (!agateCheck(vm, 1, position)) {
        agateError(vm, "Vec2I parameter expected for `position`.");
        return;
      }

      auto result = agateSlotNew<ColorClass>(vm, AGATE_RETURN_SLOT);
      *result = image->get_color(position);
    }

    static void subscript_getter2(AgateVM *vm) {
      assert(agateCheckTag<ImageClass>(vm, 0));
      auto image = agateSlotGet<ImageClass>(vm, 0);

      Vec2I position;

      if (!agateCheck(vm, 1, position.x)) {
        agateError(vm, "Int parameter expected for `x`.");
        return;
      }

      if (!agateCheck(vm, 2, position.y)) {
        agateError(vm, "Int parameter expected for `y`.");
        return;
      }

      auto result = agateSlotNew<ColorClass>(vm, AGATE_RETURN_SLOT);
      *result = image->get_color(position);
    }

    static void subscript_setter1(AgateVM *vm) {
      assert(agateCheckTag<ImageClass>(vm, 0));
      auto image = agateSlotGet<ImageClass>(vm, 0);

      Vec2I position;

      if (!agateCheck(vm, 1, position)) {
        agateError(vm, "Vec2I parameter expected for `position`.");
        return;
      }

      Color color;

      if (!agateCheck(vm, 2, color)) {
        agateError(vm, "Color parameter expected for `color`.");
        return;
      }

      image->set_color(position, color);
      agateSlotCopy(vm, AGATE_RETURN_SLOT, 2);
    }

    static void subscript_setter2(AgateVM *vm) {
      assert(agateCheckTag<ImageClass>(vm, 0));
      auto image = agateSlotGet<ImageClass>(vm, 0);

      Vec2I position;

      if (!agateCheck(vm, 1, position.x)) {
        agateError(vm, "Int parameter expected for `x`.");
        return;
      }

      if (!agateCheck(vm, 2, position.y)) {
        agateError(vm, "Int parameter expected for `y`.");
        return;
      }

      Color color;

      if (!agateCheck(vm, 3, color)) {
        agateError(vm, "Color parameter expected for `color`.");
        return;
      }

      image->set_color(position, color);
      agateSlotCopy(vm, AGATE_RETURN_SLOT, 3);
    }


  };

  /*
   * Texture
   */

  Texture::Texture(TextureKind kind, GLsizei width, GLsizei height, const uint8_t *data)
  : kind(kind)
  , id(0)
  , width(width)
  , height(height)
  , flags(0)
  , framebuffer(0)
  {
    GAMMA_GL_CHECK(glGenTextures(1, &id));

    if (id == 0) {
      return;
    }

    GLint alignment = (kind == TextureKind::COLOR) ? 4 : 1;
    GLenum format = (kind == TextureKind::COLOR) ? GL_RGBA : GL_RED;

    GAMMA_GL_CHECK(glPixelStorei(GL_UNPACK_ALIGNMENT, alignment));
    GAMMA_GL_CHECK(glBindTexture(GL_TEXTURE_2D, id));
    GAMMA_GL_CHECK(glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data));
    GAMMA_GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    GAMMA_GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
    GAMMA_GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    GAMMA_GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    GAMMA_GL_CHECK(glBindTexture(GL_TEXTURE_2D, 0));
  }

  void Texture::destroy() {
    if (framebuffer != 0) {
      glDeleteFramebuffers(1, &framebuffer);
      framebuffer = 0;
    }

    if (id != 0) {
      glDeleteTextures(1, &id);
      id = 0;
    }

    width = height = flags = 0;
  }

  GLenum Texture::compute_min_filter() const {
    if (has_mipmap()) {
      return is_smooth() ? GL_LINEAR_MIPMAP_LINEAR : GL_NEAREST_MIPMAP_LINEAR;
    }

    return is_smooth() ? GL_LINEAR : GL_NEAREST;
  }

  void Texture::make_renderable(Vec2I size) {
    if (size.x != width || size.y != height) {
      width = size.x;
      height = size.y;

      GLint alignment = (kind == TextureKind::COLOR) ? 4 : 1;
      GLenum format = (kind == TextureKind::COLOR) ? GL_RGBA : GL_RED;

      GAMMA_GL_CHECK(glPixelStorei(GL_UNPACK_ALIGNMENT, alignment));
      GAMMA_GL_CHECK(glBindTexture(GL_TEXTURE_2D, id));
      GAMMA_GL_CHECK(glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, nullptr));
    }

    GAMMA_GL_CHECK(glBindTexture(GL_TEXTURE_2D, 0));

    if (framebuffer == 0) {
      GAMMA_GL_CHECK(glGenFramebuffers(1, &framebuffer));
      GAMMA_GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, framebuffer));
      GAMMA_GL_CHECK(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, id, 0));
      assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
    } else {
      GAMMA_GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, framebuffer));
    }
  }

  struct TextureApi : TextureClass {

    static void destroy(AgateVM *vm, const char *unit_name, const char *class_name, void *data) {
      auto texture = static_cast<Texture *>(data);
      texture->destroy();
    }

    static void from_file(AgateVM *vm) {
      assert(agateCheckTag<TextureClass>(vm, 0));
      auto texture = agateSlotGet<TextureClass>(vm, 0);

      const char *filename = nullptr;

      if (!agateCheck(vm, 1, filename)) {
        agateError(vm, "String parameter expected for `filename`.");
        return;
      }

      Image image(filename);

      if (!image.loaded()) {
        agateError(vm, "Unable to load texture: '%s'.", filename);
        return;
      }

      *texture = Texture(TextureKind::COLOR, image.width, image.height, image.pixels);
      image.destroy();
    }

    static void from_image(AgateVM *vm) {
      assert(agateCheckTag<TextureClass>(vm, 0));
      auto texture = agateSlotGet<TextureClass>(vm, 0);

      if (!agateCheckTag<ImageClass>(vm, 1)) {
        agateError(vm, "Image parameter expected for `image`.");
        return;
      }

      auto image = agateSlotGet<ImageClass>(vm, 1);
      *texture = Texture(TextureKind::COLOR, image->width, image->height, image->pixels);
    }

    static void get_size(AgateVM *vm) {
      assert(agateCheckTag<TextureClass>(vm, 0));
      auto texture = agateSlotGet<TextureClass>(vm, 0);

      auto result = agateSlotNew<Vec2IClass>(vm, AGATE_RETURN_SLOT);
      result->x = texture->width;
      result->y = texture->height;
    }

    static void set_size(AgateVM *vm) {
      assert(agateCheckTag<TextureClass>(vm, 0));
      auto texture = agateSlotGet<TextureClass>(vm, 0);

      Vec2I size;

      if (!agateCheck(vm, 1, size)) {
        agateError(vm, "Vec2I parameter expected for `value`.");
        return;
      }

      texture->width = size.x;
      texture->height = size.y;

      assert(texture->kind == TextureKind::COLOR);
      glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
      glBindTexture(GL_TEXTURE_2D, texture->id);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture->width, texture->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
      glBindTexture(GL_TEXTURE_2D, 0);
    }

    static void is_smooth(AgateVM *vm) {
      assert(agateCheckTag<TextureClass>(vm, 0));
      auto texture = agateSlotGet<TextureClass>(vm, 0);
      agateSlotSetBool(vm, AGATE_RETURN_SLOT, texture->is_smooth());
    }

    static void set_smooth(AgateVM *vm) {
      assert(agateCheckTag<TextureClass>(vm, 0));
      auto texture = agateSlotGet<TextureClass>(vm, 0);

      bool smooth;

      if (!agateCheck(vm, 1, smooth)) {
        agateError(vm, "Bool parameter expected for `value`.");
        return;
      }

      if (smooth == texture->is_smooth()) {
        return;
      }

      if (smooth) {
        texture->flags |= TEXTURE_SMOOTH;
      } else {
        texture->flags &= ~TEXTURE_SMOOTH;
      }

      glBindTexture(GL_TEXTURE_2D, texture->id);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, smooth ? GL_LINEAR : GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, texture->compute_min_filter());
      glBindTexture(GL_TEXTURE_2D, 0);
    }

    static void is_repeated(AgateVM *vm) {
      assert(agateCheckTag<TextureClass>(vm, 0));
      auto texture = agateSlotGet<TextureClass>(vm, 0);
      agateSlotSetBool(vm, AGATE_RETURN_SLOT, texture->is_repeated());
    }

    static void set_repeated(AgateVM *vm) {
      assert(agateCheckTag<TextureClass>(vm, 0));
      auto texture = agateSlotGet<TextureClass>(vm, 0);

      bool repeated;

      if (!agateCheck(vm, 1, repeated)) {
        agateError(vm, "Bool parameter expected for `value`.");
        return;
      }

      if (repeated == texture->is_repeated()) {
        return;
      }

      if (repeated) {
        texture->flags |= TEXTURE_REPEATED;
      } else {
        texture->flags &= ~TEXTURE_REPEATED;
      }

      glBindTexture(GL_TEXTURE_2D, texture->id);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, repeated ? GL_REPEAT : GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, repeated ? GL_REPEAT : GL_CLAMP_TO_EDGE);
      glBindTexture(GL_TEXTURE_2D, 0);
    }

    static void generate_mipmap(AgateVM *vm) {
      assert(agateCheckTag<TextureClass>(vm, 0));
      auto texture = agateSlotGet<TextureClass>(vm, 0);

      if (texture->has_mipmap()) {
        return;
      }

      texture->flags |= TEXTURE_MIPMAP;

      glBindTexture(GL_TEXTURE_2D, texture->id);
      glGenerateMipmap(GL_TEXTURE_2D);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, texture->compute_min_filter());
      glBindTexture(GL_TEXTURE_2D, 0);

    }

  };

  /*
   * Sprite
   */

  Sprite::Sprite(const Texture& texture, AgateHandle *handle)
  : buffer(0)
  , color({ 1.0f, 1.0f, 1.0f, 1.0f })
  , id(texture.id)
  , size(vec(texture.width, texture.height))
  , region({ vec(0.0f, 0.0f), vec(1.0f, 1.0f) })
  , handle(handle)
  {
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * 4, nullptr, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    update_buffer();
  }

  void Sprite::destroy(AgateVM *vm) {
    if (buffer != 0) {
      glDeleteBuffers(1, &buffer);
      buffer = 0;
    }

    if (handle != nullptr) {
      agateReleaseHandle(vm, handle);
      handle = nullptr;
    }
  }

  void Sprite::set_texture(const Texture& new_texture, AgateHandle *new_handle) {
    id = new_texture.id;
    size = vec(new_texture.width, new_texture.height);
    handle = new_handle;
  }

  void Sprite::update_buffer() {
    assert(buffer != 0);
    RectF bounds = { vec(0.0f, 0.0f), region.size * size };

    Vertex vertices[] = {
      { compute_position(bounds, { 0.0f, 0.0f }), color, compute_texture_position(region, { 0.0f, 0.0f }) },
      { compute_position(bounds, { 0.0f, 1.0f }), color, compute_texture_position(region, { 0.0f, 1.0f }) },
      { compute_position(bounds, { 1.0f, 0.0f }), color, compute_texture_position(region, { 1.0f, 0.0f }) },
      { compute_position(bounds, { 1.0f, 1.0f }), color, compute_texture_position(region, { 1.0f, 1.0f }) },
    };

    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
  }

  void Sprite::render(Renderer& renderer, const Transform& transform) {
    RectF bounds = { vec(0.0f, 0.0f), region.size * size };

    RendererData data;
    data.primitive = GL_TRIANGLE_STRIP;
    data.count = 4;
    data.vertex_buffer = buffer;
    data.element_buffer = 0;
    data.mode = RendererMode::COLOR;
    data.texture0 = id;
    data.texture1 = 0;
    data.shader = 0;
    data.transform = transform.compute_matrix(bounds);
    renderer.draw(data);
  }

  struct SpriteApi : SpriteClass {

    static void destroy(AgateVM *vm, const char *unit_name, const char *class_name, void *data) {
      auto sprite = static_cast<Sprite *>(data);
      sprite->destroy(vm);
    }

    static void new1(AgateVM *vm) {
      assert(agateCheckTag<SpriteClass>(vm, 0));
      auto sprite = agateSlotGet<SpriteClass>(vm, 0);

      if (!agateCheckTag<TextureClass>(vm, 1)) {
        agateError(vm, "Texture parameter expected for `texture`.");
        return;
      }

      auto texture = agateSlotGet<TextureClass>(vm, 1);
      auto handle = agateSlotGetHandle(vm, 1);

      *sprite = Sprite(*texture, handle);
    }

    static void get_texture(AgateVM *vm) {
      assert(agateCheckTag<SpriteClass>(vm, 0));
      auto sprite = agateSlotGet<SpriteClass>(vm, 0);
      agateSlotSetHandle(vm, AGATE_RETURN_SLOT, sprite->handle);
    }

    static void set_texture(AgateVM *vm) {
      assert(agateCheckTag<SpriteClass>(vm, 0));
      auto sprite = agateSlotGet<SpriteClass>(vm, 0);

      if (!agateCheckTag<TextureClass>(vm, 1)) {
        agateError(vm, "Texture parameter expected for `texture`.");
        return;
      }

      if (sprite->handle != nullptr) {
        agateReleaseHandle(vm, sprite->handle);
      }

      auto texture = agateSlotGet<TextureClass>(vm, 1);
      auto handle = agateSlotGetHandle(vm, 1);

      sprite->set_texture(*texture, handle);
      sprite->update_buffer();
    }

    static void get_texture_region(AgateVM *vm) {
      assert(agateCheckTag<SpriteClass>(vm, 0));
      auto sprite = agateSlotGet<SpriteClass>(vm, 0);

      auto result = agateSlotNew<RectFClass>(vm, AGATE_RETURN_SLOT);
      *result = sprite->region;
    }

    static void set_texture_region(AgateVM *vm) {
      assert(agateCheckTag<SpriteClass>(vm, 0));
      auto sprite = agateSlotGet<SpriteClass>(vm, 0);

      if (!agateCheck(vm, 1, sprite->region)) {
        agateError(vm, "RectF parameter expected for `value`.");
        return;
      }

      sprite->update_buffer();
    }

    static void get_color(AgateVM *vm) {
      assert(agateCheckTag<SpriteClass>(vm, 0));
      auto sprite = agateSlotGet<SpriteClass>(vm, 0);

      auto result = agateSlotNew<ColorClass>(vm, AGATE_RETURN_SLOT);
      *result = sprite->color;
    }

    static void set_color(AgateVM *vm) {
      assert(agateCheckTag<SpriteClass>(vm, 0));
      auto sprite = agateSlotGet<SpriteClass>(vm, 0);

      if (!agateCheck(vm, 1, sprite->color)) {
        agateError(vm, "Color parameter expected for `value`.");
        return;
      }

      sprite->update_buffer();
    }

    static void render(AgateVM *vm) {
      assert(agateCheckTag<SpriteClass>(vm, 0));
      auto sprite = agateSlotGet<SpriteClass>(vm, 0);

      if (!agateCheckTag<RendererClass>(vm, 1)) {
        agateError(vm, "Renderer parameter expected for `renderer`.");
        return;
      }

      if (!agateCheckTag<TransformClass>(vm, 2)) {
        agateError(vm, "Transform parameter expected for `transform`.");
        return;
      }

      sprite->render(*agateSlotGet<RendererClass>(vm, 1), *agateSlotGet<TransformClass>(vm, 2));
      agateSlotSetNil(vm, AGATE_RETURN_SLOT);
    }

  };


  /*
   * SpriteUnit
   */

  void SpriteUnit::provide_support(Support & support) {
    support.add_class_handler(unit_name, ImageClass::class_name, generic_handler<ImageClass>(ImageApi::destroy));
    support.add_class_handler(unit_name, TextureClass::class_name, generic_handler<TextureClass>(TextureApi::destroy));
    support.add_class_handler(unit_name, SpriteClass::class_name, generic_handler<SpriteClass>(SpriteApi::destroy));

    support.add_method(unit_name, ImageApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "init from_file(_)", ImageApi::from_file);
    support.add_method(unit_name, ImageApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "[_]", ImageApi::subscript_getter1);
    support.add_method(unit_name, ImageApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "[_,_]", ImageApi::subscript_getter2);
    support.add_method(unit_name, ImageApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "[_]=(_)", ImageApi::subscript_setter1);
    support.add_method(unit_name, ImageApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "[_,_]=(_)", ImageApi::subscript_setter2);

    support.add_method(unit_name, TextureApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "init from_file(_)", TextureApi::from_file);
    support.add_method(unit_name, TextureApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "init from_image(_)", TextureApi::from_image);
    support.add_method(unit_name, TextureApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "size", TextureApi::get_size);
    support.add_method(unit_name, TextureApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "size=(_)", TextureApi::set_size);
    support.add_method(unit_name, TextureApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "smooth", TextureApi::is_smooth);
    support.add_method(unit_name, TextureApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "smooth=(_)", TextureApi::set_smooth);
    support.add_method(unit_name, TextureApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "repeated", TextureApi::is_repeated);
    support.add_method(unit_name, TextureApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "repeated=(_)", TextureApi::set_repeated);
    support.add_method(unit_name, TextureApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "generate_mipmap()", TextureApi::generate_mipmap);

    support.add_method(unit_name, SpriteApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "init new(_)", SpriteApi::new1);
    support.add_method(unit_name, SpriteApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "texture", SpriteApi::get_texture);
    support.add_method(unit_name, SpriteApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "texture=(_)", SpriteApi::set_texture);
    support.add_method(unit_name, SpriteApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "texture_region", SpriteApi::get_texture_region);
    support.add_method(unit_name, SpriteApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "texture_region=(_)", SpriteApi::set_texture_region);
    support.add_method(unit_name, SpriteApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "color", SpriteApi::get_color);
    support.add_method(unit_name, SpriteApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "color=(_)", SpriteApi::set_color);
    support.add_method(unit_name, SpriteApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "render(_,_)", SpriteApi::render);
  }

}
