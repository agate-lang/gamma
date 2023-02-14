#ifndef GAMMA_TEXT_H
#define GAMMA_TEXT_H

#include <map>
#include <tuple>
#include <vector>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_STROKER_H

#include "glad/glad.h"

#include "gamma_color.h"
#include "gamma_math.h"
#include "gamma_support.h"


namespace gma {

  struct TextUnit {
    static constexpr const char * unit_name = "gamma/text";
    static void provide_support(Support & support);
  };

  /*
   * Font
   */

  struct Glyph {
    RectF bounds;
    RectF texture_rect;
    float advance = 0.0f;
  };

  struct GlyphCache {
    FT_UInt size;
    GLuint texture;
    std::map<std::tuple<uint32_t, float>, Glyph> glyphs;
    struct {
      int top = 0;
      int bottom = 0;
      int right = 0;
    } packing;
  };

  struct FontCache {
    std::vector<GlyphCache> cache_by_size;
  };

  struct Font {
    static FT_Library library;
    FT_Stroker stroker;

    FT_Face face;
    FT_UInt current_size;
    FontCache *cache;

    void destroy();

    GLuint get_texture(FT_UInt size);

    Glyph compute_glyph(uint32_t codepoint, FT_UInt size, float outline_thickness);
    float compute_kerning(uint32_t left, uint32_t right, FT_UInt size);

    GlyphCache& get_glyph_cache(FT_UInt size);
    GlyphCache create_cache(FT_UInt size);

    Glyph create_glyph(uint32_t codepoint, FT_UInt size, float outline_thickness, GlyphCache& cache);
    void set_character_size(FT_UInt size);

    float compute_line_spacing(FT_UInt size);

    static const char *error_message(FT_Error error);
  };

  struct FontClass : TextUnit {
    using type = Font;
    static constexpr const char * class_name = "Font";
    static constexpr uint64_t tag = compute_tag(unit_name, class_name);
  };


  /*
   * Alignment
   */

  enum class TextAlignement {
    NONE,
    LEFT,
    RIGHT,
    JUSTIFY,
    CENTER,
  };

  struct AlignmentClass : TextUnit {
    // no type
    static constexpr const char * class_name = "Alignment";
    // no tag
  };

  /*
   * Text
   */

  struct Renderer;
  struct Transform;

  struct Text {
    Font *font;
    AgateHandle *font_handle;

    const char *string;
    AgateHandle *string_handle;

    int character_size;
    Color color;

    float outline_thickness;
    Color outline_color;

    float line_spacing;
    float letter_spacing;
    float paragraph_width;
    TextAlignement alignment;

    RectF bounds;

    GLuint buffer;
    GLsizei buffer_count;
    GLuint outline_buffer;
    GLsizei outline_buffer_count;

    void update_buffer();
    void render(Renderer& renderer, const Transform& transform);
  };

  struct TextClass : TextUnit {
    using type = Text;
    static constexpr const char * class_name = "Text";
    static constexpr uint64_t tag = compute_tag(unit_name, class_name);
  };

}

#endif // GAMMA_TEXT_H
