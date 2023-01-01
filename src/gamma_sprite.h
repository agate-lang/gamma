#ifndef GAMMA_SPRITE_H
#define GAMMA_SPRITE_H

#include <cstdint>

#include "glad/glad.h"

#include "gamma_color.h"
#include "gamma_math.h"
#include "gamma_support.h"

namespace gma {

  struct SpriteUnit {
    static constexpr const char * unit_name = "gamma/sprite";
    static void provide_support(Support & support);
  };

  /*
   *
   */

  struct Image {
    int width;
    int height;
    uint8_t *pixels;

    Image() = default;
    Image(const char *filename);
    void destroy();

    bool loaded() const {
      return width != 0 && height != 0 && pixels != nullptr;
    }

    Color get_color(Vec2I position) const;
    void set_color(Vec2I position, Color color);
  };

  struct ImageClass : SpriteUnit {
    using type = Image;
    static constexpr const char * class_name = "Image";
    static constexpr uint64_t tag = compute_tag(unit_name, class_name);
  };

  /*
   * Texture
   */

  inline constexpr uint32_t TEXTURE_SMOOTH    = 0x01;
  inline constexpr uint32_t TEXTURE_REPEATED  = 0x02;
  inline constexpr uint32_t TEXTURE_MIPMAP    = 0x04;

  enum class TextureKind {
    COLOR,
    ALPHA,
  };

  struct Texture {
    TextureKind kind;
    GLuint id;
    GLsizei width;
    GLsizei height;
    uint32_t flags;

    Texture() = default;
    Texture(TextureKind kind, GLsizei width, GLsizei height, const uint8_t *data);
    void destroy();

    bool is_smooth() const {
      return (flags & TEXTURE_SMOOTH) != 0;
    }

    bool is_repeated() const {
      return (flags & TEXTURE_REPEATED) != 0;
    }

    bool has_mipmap() const {
      return (flags & TEXTURE_MIPMAP) != 0;
    }

    GLenum compute_min_filter() const;
  };

  struct TextureClass : SpriteUnit {
    using type = Texture;
    static constexpr const char * class_name = "Texture";
    static constexpr uint64_t tag = compute_tag(unit_name, class_name);
  };

  /*
   * Sprite
   */
  struct Renderer;
  struct Transform;

  struct Sprite {
    GLuint buffer;
    Color color;
    // texture
    GLuint id;
    Vec2I size;
    RectF region;
    AgateHandle *handle;

    Sprite() = default;
    Sprite(const Texture& texture, AgateHandle *handle);
    void destroy(AgateVM *vm);

    void set_texture(const Texture& new_texture, AgateHandle *new_handle);

    void update_buffer();
    void render(Renderer& renderer, const Transform& transform);
  };

  struct SpriteClass : SpriteUnit {
    using type = Sprite;
    static constexpr const char * class_name = "Sprite";
    static constexpr uint64_t tag = compute_tag(unit_name, class_name);
  };

}

#endif // GAMMA_SPRITE_H
