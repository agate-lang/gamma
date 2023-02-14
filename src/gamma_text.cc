#include "gamma_text.h"

#include <algorithm>

#include "gamma_agate.h"
#include "gamma_debug.h"
#include "gamma_render.h"

namespace gma {

  static constexpr float Scale = (1 << 6);

  float convert(FT_Pos value) {
    return static_cast<float>(value) / Scale;
  }

  /*
   * Font
   */

  FT_Library Font::library = nullptr;

  void Font::destroy() {
    delete cache;
    cache = nullptr;

    if (face != nullptr) {
      if (FT_Error err; (err = FT_Done_Face(face)) != 0) {
      }

      face = nullptr;
    }

    if (stroker != nullptr) {
      FT_Stroker_Done(stroker);
      stroker = nullptr;
    }
  }

  GLuint Font::get_texture(FT_UInt size) {
    GlyphCache& cache = get_glyph_cache(size);
    return cache.texture;
  }

  Glyph Font::compute_glyph(uint32_t codepoint, FT_UInt size, float outline_thickness) {
    GlyphCache& cache = get_glyph_cache(size);

    auto key = std::make_tuple(codepoint, outline_thickness);
    auto it = cache.glyphs.find(key);

    if (it == cache.glyphs.end()) {
      std::tie(it, std::ignore) = cache.glyphs.insert(std::make_pair(key, create_glyph(codepoint, size, outline_thickness, cache)));
    }

    return it->second;
  }

  float Font::compute_kerning(uint32_t left, uint32_t right, FT_UInt size) {
    if (left == 0 || right == 0) {
      return 0.0f;
    }

    set_character_size(size);

    if (!FT_HAS_KERNING(face)) {
      return 0.0f;
    }

    auto left_index = FT_Get_Char_Index(face, left);
    auto right_index = FT_Get_Char_Index(face, right);

    FT_Vector kerning;

    if (FT_Error err; (err = FT_Get_Kerning(face, left_index, right_index, FT_KERNING_UNFITTED, &kerning)) != 0) {
      // TODO
    }

    return convert(kerning.x);
  }

  GlyphCache& Font::get_glyph_cache(FT_UInt size) {
    auto & cache_by_size = cache->cache_by_size;

    auto it = std::find_if(cache_by_size.begin(), cache_by_size.end(), [size](const GlyphCache& glyph_cache) {
      return glyph_cache.size == size;
    });

    if (it != cache_by_size.end()) {
      return *it;
    }

    cache_by_size.push_back(create_cache(size));
    return cache_by_size.back();
  }

  static constexpr int TextureSize = 4096;
  static constexpr int Padding = 1;

  GlyphCache Font::create_cache(FT_UInt size) {
    GlyphCache cache;
    cache.size = size;

    GAMMA_GL_CHECK(glGenTextures(1, &cache.texture));

    GAMMA_GL_CHECK(glBindTexture(GL_TEXTURE_2D, cache.texture));
    GAMMA_GL_CHECK(glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, TextureSize, TextureSize, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr));

    GAMMA_GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    GAMMA_GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
    GAMMA_GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    GAMMA_GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));

    GAMMA_GL_CHECK(glBindTexture(GL_TEXTURE_2D, 0));

    return cache;
  }

  Glyph Font::create_glyph(uint32_t codepoint, FT_UInt size, float outline_thickness, GlyphCache& cache) {
    Glyph result;

    set_character_size(size);

    FT_Int32 flags = FT_LOAD_TARGET_NORMAL | FT_LOAD_FORCE_AUTOHINT;

    if (outline_thickness > 0) {
      flags |= FT_LOAD_NO_BITMAP;
    }

    if (FT_Error err; (err = FT_Load_Char(face, codepoint, flags)) != 0) {
      // TODO
      return result;
    }

    FT_GlyphSlot slot = face->glyph;
    FT_Glyph glyph;

    if (FT_Error err; (err = FT_Get_Glyph(slot, &glyph)) != 0) {
      return result;
    }

    if (outline_thickness > 0) {
      assert(glyph->format == FT_GLYPH_FORMAT_OUTLINE);
      FT_Stroker_Set(stroker, static_cast<FT_Fixed>(outline_thickness * Scale), FT_STROKER_LINECAP_ROUND, FT_STROKER_LINEJOIN_ROUND, 0);
      FT_Glyph_Stroke(&glyph, stroker, 0);
    }

    if (FT_Error err; (err = FT_Glyph_To_Bitmap(&glyph, FT_RENDER_MODE_NORMAL, nullptr, 1)) != 0) {
      // TODO
      FT_Done_Glyph(glyph);
      return result;
    }

    assert(glyph->format == FT_GLYPH_FORMAT_BITMAP);
    FT_BitmapGlyph bglyph = reinterpret_cast<FT_BitmapGlyph>(glyph);

    // advance

    result.advance = convert(slot->metrics.horiAdvance);

    // size

    Vec2I glyph_size = vec<int>(bglyph->bitmap.width, bglyph->bitmap.rows);

    if (glyph_size.x == 0 || glyph_size.y == 0) {
      FT_Done_Glyph(glyph);
      return result;
    }

    glyph_size += 2 * Padding;

    if (cache.packing.right + glyph_size.x > TextureSize) {
      cache.packing.right = 0;
      cache.packing.top = cache.packing.bottom;
    }

    assert(cache.packing.top + glyph_size.y <= TextureSize);

    Vec2I glyph_position = vec(cache.packing.right, cache.packing.top);

    if (glyph_position.y + glyph_size.y > cache.packing.bottom) {
      cache.packing.bottom = glyph_position.y + glyph_size.y;
    }

    cache.packing.right += glyph_size.x;

    result.texture_rect.position = (glyph_position + Padding) / static_cast<float>(TextureSize);
    result.texture_rect.size = (glyph_size - 2 * Padding) / static_cast<float>(TextureSize);

    // bounds

    if (outline_thickness == 0.0f) {
      result.bounds.position = vec(convert(slot->metrics.horiBearingX), - convert(slot->metrics.horiBearingY));
      result.bounds.size = vec(convert(slot->metrics.width), convert(slot->metrics.height));
    } else {
      result.bounds.position = vec(static_cast<float>(bglyph->left), - static_cast<float>(bglyph->top));
      result.bounds.size = vec(static_cast<float>(bglyph->bitmap.width), static_cast<float>(bglyph->bitmap.rows));
    }

    // bitmap

    std::vector<uint8_t> buffer(glyph_size.x * glyph_size.y, 0);
    auto source = bglyph->bitmap.buffer;

    for (int y = Padding; y < glyph_size.y - Padding; ++y) {
      for (int x = Padding; x < glyph_size.x - Padding; ++x) {
        buffer[y * glyph_size.x + x] = source[x - Padding];
      }

      source += bglyph->bitmap.pitch;
    }

    GAMMA_GL_CHECK(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));
    GAMMA_GL_CHECK(glBindTexture(GL_TEXTURE_2D, cache.texture));
    GAMMA_GL_CHECK(glTexSubImage2D(GL_TEXTURE_2D, 0, glyph_position.x, glyph_position.y, glyph_size.x, glyph_size.y, GL_RED, GL_UNSIGNED_BYTE, buffer.data()));
    GAMMA_GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_A, GL_RED));
    GAMMA_GL_CHECK(glBindTexture(GL_TEXTURE_2D, 0));

    FT_Done_Glyph(glyph);
    return result;
  }

  void Font::set_character_size(FT_UInt size) {
    if (current_size == size) {
      return;
    }

    if (FT_Error err; (err = FT_Set_Pixel_Sizes(face, 0, size)) != 0) {
      std::printf("Error in set_character_size\n");
      // TODO
    }

    current_size = size;
  }

  float Font::compute_line_spacing(FT_UInt size) {
    set_character_size(size);
    return convert(face->size->metrics.height);
  }

  const char *Font::error_message(FT_Error error) {
    switch (error) {

      #undef FTERRORS_H_
      #define FT_ERRORDEF(e, v, s)  \
      case v:                       \
        return s;
      #include FT_ERRORS_H

      default:
        break;
    };

    return "unknown error";
  }

  struct FontApi : FontClass {

    static void destroy(AgateVM *vm, const char *unit_name, const char *class_name, void *data) {
      auto font = static_cast<Font *>(data);
      font->destroy();
    }

    static void from_file(AgateVM *vm) {
      assert(agateCheckTag<FontClass>(vm, 0));
      auto font = agateSlotGet<FontClass>(vm, 0);

      const char *filename = nullptr;

      if (!agateCheck(vm, 1, filename)) {
        agateError(vm, "String parameter expected for `filename`.");
        return;
      }

      font->stroker = nullptr;
      font->face = nullptr;
      font->current_size = 0;
      font->cache = nullptr;

      assert(Font::library != nullptr);

      if (FT_Error err; (err = FT_Stroker_New(Font::library, &font->stroker)) != 0) {
        agateError(vm, "Could not create stroker.");
        return;
      }

      if (FT_Error err; (err = FT_New_Face(Font::library, filename, 0, &font->face)) != 0) {
        agateError(vm, "Could not create face.");
        return;
      }

      font->cache = new FontCache;
    }

  };

  /*
   * Text
   */

  struct CodepointRange {
    std::string_view ref;

    struct Iterator {
      using difference_type = std::ptrdiff_t;
      using value_type = uint32_t;
      using pointer = value_type;
      using reference = value_type;
      using iterator_category = std::forward_iterator_tag;

      void swap(Iterator& other) noexcept { std::swap(current, other.current); }
      constexpr reference operator*() const noexcept { return decode(); }
      constexpr pointer operator->() const noexcept { return decode(); }
      constexpr Iterator& operator++() noexcept { step(); return *this; }
      constexpr Iterator operator++(int) noexcept {
        Iterator copy = *this;
        step();
        return copy;
      }
      constexpr bool operator!=(const Iterator& other) const noexcept { return current != other.current; }
      constexpr bool operator==(const Iterator& other) const noexcept { return current == other.current; }

      const char *current;

    private:
      constexpr uint32_t decode() const noexcept {
        uint32_t codepoint = 0;
        uint8_t c = current[0];

        if ((c & 0b10000000) == 0b00000000) {
          codepoint = c & 0b01111111;
        } else if ((c & 0b11100000) == 0b11000000) {
          codepoint = c & 0b00011111;
          codepoint = (codepoint << 6) + (current[1] & 0b00111111);
        } else if ((c & 0b11110000) == 0b11100000) {
          codepoint = c & 0b00001111;
          codepoint = (codepoint << 6) + (current[1] & 0b00111111);
          codepoint = (codepoint << 6) + (current[2] & 0b00111111);
        } else {
          assert((c & 0b11111000) == 0b11110000);
          codepoint = c & 0b00000111;
          codepoint = (codepoint << 6) + (current[1] & 0b00111111);
          codepoint = (codepoint << 6) + (current[2] & 0b00111111);
          codepoint = (codepoint << 6) + (current[3] & 0b00111111);
        }

        return codepoint;
      }

      constexpr void step() noexcept {
        uint8_t c = current[0];

        if ((c & 0b10000000) == 0b00000000) {
          current += 1;
        } else if ((c & 0b11100000) == 0b11000000) {
          current += 2;
        } else if ((c & 0b11110000) == 0b11100000) {
          current += 3;
        } else {
          assert((c & 0b11111000) == 0b11110000);
          current += 4;
        }
      }
    };

    Iterator begin() const noexcept {
      return Iterator{ ref.data() };
    }

    Iterator end() const noexcept {
      return Iterator{ ref.data() + ref.size() };
    }
  };

  constexpr CodepointRange codepoints(std::string_view ref) {
    return CodepointRange{ ref };
  }

  bool is_delimiter(char c, std::string_view delimiters) {
    for (auto d : delimiters) {
      if (c == d) {
        return true;
      }
    }

    return false;
  }


  std::vector<std::string_view> split(std::string_view str, std::string_view delimiters) {
    std::size_t sz = str.size();
    std::size_t i = 0;
    std::vector<std::string_view> result;

    while (i < sz) {
      while (i < sz && is_delimiter(str[i], delimiters)) {
        ++i;
      }

      if (i == sz) {
        break;
      }

      const char *start = str.data() + i;
      size_t len = 0;

      while (i < sz && !is_delimiter(str[i], delimiters)) {
        ++i;
        ++len;
      }

      result.push_back(std::string_view(start, len));
    }

    return result;
  }

  std::vector<std::string_view> split_in_paragraphs(std::string_view str) {
    return split(str, "\n");
  }

  std::vector<std::string_view> split_in_words(std::string_view str) {
    return split(str, " \t");
  }

  struct ParagraphLine {
    std::vector<std::string_view> words;
    float indent = 0.0f;
    float spacing = 0.0f;
  };

  struct Paragraph {
    std::vector<ParagraphLine> lines;
  };

  float compute_word_width(std::string_view word, unsigned character_size, Font& font) {
    assert(character_size > 0);
    assert(!word.empty());

    float width = 0.0f;
    uint32_t prev_codepoint = '\0';

    for (uint32_t curr_codepoint : codepoints(word)) {
      width += font.compute_kerning(prev_codepoint, curr_codepoint, character_size);
      prev_codepoint = curr_codepoint;

      auto glyph = font.compute_glyph(curr_codepoint, character_size, 0.0f);
      width += glyph.advance;
    }

    return width;
  }

  std::vector<Paragraph> make_paragraphs(const char *str, float space_width, float paragraph_width, TextAlignement align, unsigned character_size, Font& font) {
    std::vector<std::string_view> paragraphs = split_in_paragraphs(str);
    std::vector<Paragraph> result;

    for (auto raw_paragraph : paragraphs) {
      std::vector<std::string_view> words = split_in_words(raw_paragraph);

      Paragraph paragraph;

      if (align == TextAlignement::NONE) {
        ParagraphLine line;
        line.words = std::move(words);
        line.indent = 0.0f;
        line.spacing = space_width;
        paragraph.lines.push_back(std::move(line));
      } else {
        ParagraphLine current_line;
        float current_width = 0.0f;

        for (auto word : words) {
          float word_width = compute_word_width(word, character_size, font);

          if (!current_line.words.empty() && current_width + space_width + word_width > paragraph_width) {
            auto world_count = current_line.words.size();

            switch (align) {
              case TextAlignement::LEFT:
                current_line.indent = 0.0f;
                current_line.spacing = space_width;
                break;

              case TextAlignement::RIGHT:
                current_line.indent = paragraph_width - current_width;
                current_line.spacing = space_width;
                break;

              case TextAlignement::CENTER:
                current_line.indent = (paragraph_width - current_width) / 2;
                current_line.spacing = space_width;
                break;

              case TextAlignement::JUSTIFY:
                current_line.indent = 0.0f;

                if (world_count > 1) {
                  current_line.spacing = space_width + (paragraph_width - current_width) / (world_count - 1);
                } else {
                  current_line.spacing = 0.0f;
                }

                break;

              case TextAlignement::NONE:
                assert(false);
                break;
            }

            paragraph.lines.push_back(std::move(current_line));
            current_line.words.clear();
          }

          if (current_line.words.empty()) {
            current_width = word_width;
          } else {
            current_width += space_width + word_width;
          }

          current_line.words.push_back(word);
        }

        // add the last line
        if (!current_line.words.empty()) {
          switch (align) {
            case TextAlignement::LEFT:
            case TextAlignement::JUSTIFY:
              current_line.indent = 0.0f;
              current_line.spacing = space_width;
              break;

            case TextAlignement::RIGHT:
              current_line.indent = paragraph_width - current_width;
              current_line.spacing = space_width;
              break;

            case TextAlignement::CENTER:
              current_line.indent = (paragraph_width - current_width) / 2;
              current_line.spacing = space_width;
              break;

            case TextAlignement::NONE:
              assert(false);
              break;
          }

          paragraph.lines.push_back(std::move(current_line));
        }
      }

      result.push_back(std::move(paragraph));
      paragraph.lines.clear();
    }

    return result;
  }

  void Text::update_buffer() {
    if (character_size == 0) {
      return;
    }

    std::vector<Vertex> vertices;
    std::vector<Vertex> outline_vertices;

    auto add_glyph_vertex = [](std::vector<Vertex>& array, const Glyph& glyph, Vec2F position) {
      Vertex vertices[4];

      vertices[0].position = position + compute_position(glyph.bounds, { 0.0f, 0.0f });
      vertices[1].position = position + compute_position(glyph.bounds, { 0.0f, 1.0f });
      vertices[2].position = position + compute_position(glyph.bounds, { 1.0f, 0.0f });
      vertices[3].position = position + compute_position(glyph.bounds, { 1.0f, 1.0f });

      vertices[0].texcoords = compute_position(glyph.texture_rect, { 0.0f, 0.0f });
      vertices[1].texcoords = compute_position(glyph.texture_rect, { 0.0f, 1.0f });
      vertices[2].texcoords = compute_position(glyph.texture_rect, { 1.0f, 0.0f });
      vertices[3].texcoords = compute_position(glyph.texture_rect, { 1.0f, 1.0f });

      // first triangle
      array.push_back(vertices[0]);
      array.push_back(vertices[1]);
      array.push_back(vertices[2]);

      // second triangle
      array.push_back(vertices[2]);
      array.push_back(vertices[1]);
      array.push_back(vertices[3]);
    };

    bounds = { 0.0f, 0.0f, 0.0f, 0.0f };

    float space_width = font->compute_glyph(' ', character_size, 0.0f).advance;
    float additional_space = (space_width / 3) * (letter_spacing - 1.0f);
    space_width += additional_space;
    float line_height = font->compute_line_spacing(character_size) * line_spacing;

    auto paragraphs = make_paragraphs(string, space_width, paragraph_width, alignment, character_size, *font);
    Vec2F position = { 0.0f, 0.0f };
    Vec2F min = position;
    Vec2F max = position;

    for (auto paragraph : paragraphs) {
      for (const auto & line : paragraph.lines) {
        position.x = line.indent;

        for (auto word : line.words) {
          uint32_t prev_codepoint = '\0';

          for (auto curr_codepoint : codepoints(word)) {
            position.x += font->compute_kerning(prev_codepoint, curr_codepoint, character_size);
            prev_codepoint = curr_codepoint;

            if (outline_thickness > 0) {
              auto glyph = font->compute_glyph(curr_codepoint, character_size, outline_thickness);
              add_glyph_vertex(outline_vertices, glyph, position);

              auto top_left = compute_position(glyph.bounds, { 0.0f, 0.0f });
              min.x = std::min(min.x, position.x + top_left.x);
              min.y = std::min(min.y, position.y + top_left.y);

              auto bottom_right = compute_position(glyph.bounds, { 1.0f, 1.0f });
              max.x = std::max(max.x, position.x + bottom_right.x);
              max.y = std::max(max.y, position.y + bottom_right.y);
            }

            auto glyph = font->compute_glyph(curr_codepoint, character_size, 0.0f);
            add_glyph_vertex(vertices, glyph, position);

            if (outline_thickness == 0.0f) {
              auto top_left = compute_position(glyph.bounds, { 0.0f, 0.0f });
              min.x = std::min(min.x, position.x + top_left.x);
              min.y = std::min(min.y, position.y + top_left.y);

              auto bottom_right = compute_position(glyph.bounds, { 1.0f, 1.0f });
              max.x = std::max(max.x, position.x + bottom_right.x);
              max.y = std::max(max.y, position.y + bottom_right.y);
            }

            position.x += glyph.advance + additional_space;
          }

          position.x += line.spacing;
        }

        position.y += line_height;
      }
    }

    bounds = { min, max - min };

    if (alignment != TextAlignement::NONE && paragraph_width > 0.0f) {
      bounds.position.x = 0.0f;
      bounds.size.x = paragraph_width;
    }

    for (auto & vertex : vertices) {
      vertex.color = color;
    }

    for (auto & vertex : outline_vertices) {
      vertex.color = outline_color;
    }

    GAMMA_GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, buffer));

    if (vertices.size() == static_cast<std::size_t>(buffer_count)) {
      GAMMA_GL_CHECK(glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(Vertex), vertices.data()));
    } else {
      GAMMA_GL_CHECK(glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW));
      buffer_count = static_cast<GLsizei>(vertices.size());
    }

    GAMMA_GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, 0));

    assert((outline_thickness == 0.0f) == outline_vertices.empty());

    if (!outline_vertices.empty()) {
      assert(outline_vertices.size() == vertices.size());
      GAMMA_GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, outline_buffer));

      if (vertices.size() == static_cast<std::size_t>(outline_buffer_count)) {
        GAMMA_GL_CHECK(glBufferSubData(GL_ARRAY_BUFFER, 0, outline_vertices.size() * sizeof(Vertex), outline_vertices.data()));
      } else {
        GAMMA_GL_CHECK(glBufferData(GL_ARRAY_BUFFER, outline_vertices.size() * sizeof(Vertex), outline_vertices.data(), GL_STATIC_DRAW));
        outline_buffer_count = static_cast<GLsizei>(outline_vertices.size());
      }

      GAMMA_GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, 0));
    }

  }

  void Text::render(Renderer& renderer, const Transform& transform) {
    RendererData data;
    data.primitive = GL_TRIANGLES;
    data.count = buffer_count;
    data.element_buffer = 0;
    data.mode = RendererMode::ALPHA;
    data.texture0 = font->get_texture(character_size);
    data.texture1 = 0;
    data.shader = 0;
    data.transform = transform.compute_matrix(bounds);

    if (outline_thickness > 0) {
      data.vertex_buffer = outline_buffer;
      renderer.draw(data);
    }

    data.vertex_buffer = buffer;
    renderer.draw(data);
  }


  struct TextApi : TextClass {
    static void destroy(AgateVM *vm, const char *unit_name, const char *class_name, void *data) {
      auto text = static_cast<Text *>(data);

      if (text->font_handle != nullptr) {
        agateReleaseHandle(vm, text->font_handle);
        text->font_handle = nullptr;
      }

      if (text->string_handle != nullptr) {
        agateReleaseHandle(vm, text->string_handle);
        text->string_handle = nullptr;
      }

      if (text->buffer != 0) {
        GAMMA_GL_CHECK(glDeleteBuffers(1, &text->buffer));
        text->buffer = 0;
      }

      if (text->outline_buffer != 0) {
        GAMMA_GL_CHECK(glDeleteBuffers(1, &text->outline_buffer));
        text->outline_buffer = 0;
      }

    }

    static void new3(AgateVM *vm) {
      assert(agateCheckTag<TextClass>(vm, 0));
      auto text = agateSlotGet<TextClass>(vm, 0);

      if (!agateCheckTag<FontClass>(vm, 1)) {
        agateError(vm, "Font parameter expected for `font`.");
        return;
      }

      text->font = agateSlotGet<FontClass>(vm, 1);
      text->font_handle = agateSlotGetHandle(vm, 1);

      if (!agateCheck(vm, 2, text->string)) {
        agateError(vm, "String parameter expected for `string`.");
        return;
      }

      text->string_handle = agateSlotGetHandle(vm, 2);

      if (!agateCheck(vm, 3, text->character_size)) {
        agateError(vm, "Int parameter expected for `size`.");
        return;
      }

      text->color = { 0.0f, 0.0f, 0.0f, 1.0f };

      text->outline_thickness = 0.0f;
      text->outline_color = { 0.0f, 0.0f, 0.0f, 1.0f };

      text->line_spacing = 1.0f;
      text->letter_spacing = 1.0f;
      text->paragraph_width = 0.0f;
      text->alignment = TextAlignement::NONE;

      GAMMA_GL_CHECK(glGenBuffers(1, &text->buffer));
      text->buffer_count = 0;

      GAMMA_GL_CHECK(glGenBuffers(1, &text->outline_buffer));
      text->outline_buffer_count = 0;

      text->update_buffer();
    }

    static void get_font(AgateVM *vm) {
      assert(agateCheckTag<TextClass>(vm, 0));
      auto text = agateSlotGet<TextClass>(vm, 0);
      agateSlotSetHandle(vm, AGATE_RETURN_SLOT, text->font_handle);
    }

    static void set_font(AgateVM *vm) {
      assert(agateCheckTag<TextClass>(vm, 0));
      auto text = agateSlotGet<TextClass>(vm, 0);

      if (!agateCheckTag<FontClass>(vm, 1)) {
        agateError(vm, "Font parameter expected for `value`.");
        return;
      }

      auto font = agateSlotGet<FontClass>(vm, 1);

      text->font = font;
      agateReleaseHandle(vm, text->font_handle);
      text->font_handle = agateSlotGetHandle(vm, 1);

      text->update_buffer();
    }

    static void get_string(AgateVM *vm) {
      assert(agateCheckTag<TextClass>(vm, 0));
      auto text = agateSlotGet<TextClass>(vm, 0);
      agateSlotSetHandle(vm, AGATE_RETURN_SLOT, text->string_handle);
    }

    static void set_string(AgateVM *vm) {
      assert(agateCheckTag<TextClass>(vm, 0));
      auto text = agateSlotGet<TextClass>(vm, 0);

      const char *string = nullptr;

      if (!agateCheck(vm, 1, string)) {
        agateError(vm, "String parameter expected for `value`.");
        return;
      }

      text->string = string;
      agateReleaseHandle(vm, text->string_handle);
      text->string_handle = agateSlotGetHandle(vm, 1);

      text->update_buffer();
    }

    static void get_size(AgateVM *vm) {
      assert(agateCheckTag<TextClass>(vm, 0));
      auto text = agateSlotGet<TextClass>(vm, 0);
      agateSlotSetInt(vm, AGATE_RETURN_SLOT, text->character_size);
    }

    static void set_size(AgateVM *vm) {
      assert(agateCheckTag<TextClass>(vm, 0));
      auto text = agateSlotGet<TextClass>(vm, 0);

      if (!agateCheck(vm, 1, text->character_size)) {
        agateError(vm, "Int parameter expected for `value`.");
        return;
      }

      text->update_buffer();
    }

    static void get_color(AgateVM *vm) {
      assert(agateCheckTag<TextClass>(vm, 0));
      auto text = agateSlotGet<TextClass>(vm, 0);

      auto result = agateSlotNew<ColorClass>(vm, AGATE_RETURN_SLOT);
      *result = text->color;
    }

    static void set_color(AgateVM *vm) {
      assert(agateCheckTag<TextClass>(vm, 0));
      auto text = agateSlotGet<TextClass>(vm, 0);

      if (!agateCheck(vm, 1, text->color)) {
        agateError(vm, "Color parameter expected for `value`.");
        return;
      }

      text->update_buffer();
    }

    static void get_outline_thickness(AgateVM *vm) {
      assert(agateCheckTag<TextClass>(vm, 0));
      auto text = agateSlotGet<TextClass>(vm, 0);
      agateSlotSetFloat(vm, AGATE_RETURN_SLOT, text->outline_thickness);
    }

    static void set_outline_thickness(AgateVM *vm) {
      assert(agateCheckTag<TextClass>(vm, 0));
      auto text = agateSlotGet<TextClass>(vm, 0);

      if (!agateCheck(vm, 1, text->outline_thickness)) {
        agateError(vm, "Float parameter expected for `value`.");
        return;
      }

      text->update_buffer();
    }

    static void get_outline_color(AgateVM *vm) {
      assert(agateCheckTag<TextClass>(vm, 0));
      auto text = agateSlotGet<TextClass>(vm, 0);

      auto result = agateSlotNew<ColorClass>(vm, AGATE_RETURN_SLOT);
      *result = text->outline_color;
    }

    static void set_outline_color(AgateVM *vm) {
      assert(agateCheckTag<TextClass>(vm, 0));
      auto text = agateSlotGet<TextClass>(vm, 0);

      if (!agateCheck(vm, 1, text->outline_color)) {
        agateError(vm, "Color parameter expected for `value`.");
        return;
      }

      text->update_buffer();
    }

    static void get_line_spacing(AgateVM *vm) {
      assert(agateCheckTag<TextClass>(vm, 0));
      auto text = agateSlotGet<TextClass>(vm, 0);
      agateSlotSetFloat(vm, AGATE_RETURN_SLOT, text->line_spacing);
    }

    static void set_line_spacing(AgateVM *vm) {
      assert(agateCheckTag<TextClass>(vm, 0));
      auto text = agateSlotGet<TextClass>(vm, 0);

      if (!agateCheck(vm, 1, text->line_spacing)) {
        agateError(vm, "Float parameter expected for `value`.");
        return;
      }

      text->update_buffer();
    }

    static void get_letter_spacing(AgateVM *vm) {
      assert(agateCheckTag<TextClass>(vm, 0));
      auto text = agateSlotGet<TextClass>(vm, 0);
      agateSlotSetFloat(vm, AGATE_RETURN_SLOT, text->letter_spacing);
    }

    static void set_letter_spacing(AgateVM *vm) {
      assert(agateCheckTag<TextClass>(vm, 0));
      auto text = agateSlotGet<TextClass>(vm, 0);

      if (!agateCheck(vm, 1, text->letter_spacing)) {
        agateError(vm, "Float parameter expected for `value`.");
        return;
      }

      text->update_buffer();
    }

    static void get_paragraph_width(AgateVM *vm) {
      assert(agateCheckTag<TextClass>(vm, 0));
      auto text = agateSlotGet<TextClass>(vm, 0);
      agateSlotSetFloat(vm, AGATE_RETURN_SLOT, text->paragraph_width);
    }

    static void set_paragraph_width(AgateVM *vm) {
      assert(agateCheckTag<TextClass>(vm, 0));
      auto text = agateSlotGet<TextClass>(vm, 0);

      if (!agateCheck(vm, 1, text->paragraph_width)) {
        agateError(vm, "Float parameter expected for `value`.");
        return;
      }

      text->update_buffer();
    }

    static void get_alignment(AgateVM *vm) {
      assert(agateCheckTag<TextClass>(vm, 0));
      auto text = agateSlotGet<TextClass>(vm, 0);
      agateSlotSetInt(vm, AGATE_RETURN_SLOT, static_cast<int64_t>(text->alignment));
    }

    static void set_alignment(AgateVM *vm) {
      assert(agateCheckTag<TextClass>(vm, 0));
      auto text = agateSlotGet<TextClass>(vm, 0);

      int64_t raw;

      if (!agateCheck(vm, 1, raw)) {
        agateError(vm, "Int parameter expected for `value`.");
        return;
      }

      text->alignment = static_cast<TextAlignement>(raw);
      text->update_buffer();
    }

  };

  /*
   * Alignment
   */

// NAME
#define GAMMA_ALIGNMENT_LIST \
  X(NONE)     \
  X(LEFT)     \
  X(RIGHT)    \
  X(JUSTIFY)  \
  X(CENTER)

  struct AlignmentApi : AlignmentClass {
    #define X(name)                           \
    static void name(AgateVM *vm) {                 \
      agateSlotSetInt(vm, AGATE_RETURN_SLOT, static_cast<int64_t>(TextAlignement::name)); \
    }

    GAMMA_ALIGNMENT_LIST

    #undef X
  };

  /*
   * TextUnit
   */

  void TextUnit::provide_support(Support & support) {
    support.add_class_handler(unit_name, FontClass::class_name, generic_handler<FontClass>(FontApi::destroy));
    support.add_class_handler(unit_name, TextApi::class_name, generic_handler<TextClass>(TextApi::destroy));

    support.add_method(unit_name, FontApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "init from_file(_)", FontApi::from_file);

    support.add_method(unit_name, TextApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "init new(_,_,_)", TextApi::new3);
    support.add_method(unit_name, TextApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "font", TextApi::get_font);
    support.add_method(unit_name, TextApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "font=(_)", TextApi::set_font);
    support.add_method(unit_name, TextApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "string", TextApi::get_string);
    support.add_method(unit_name, TextApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "string=(_)", TextApi::set_string);
    support.add_method(unit_name, TextApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "size", TextApi::get_size);
    support.add_method(unit_name, TextApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "size=(_)", TextApi::set_size);
    support.add_method(unit_name, TextApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "color", TextApi::get_color);
    support.add_method(unit_name, TextApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "color=(_)", TextApi::set_color);
    support.add_method(unit_name, TextApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "outline_thickness", TextApi::get_outline_thickness);
    support.add_method(unit_name, TextApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "outline_thickness=(_)", TextApi::set_outline_thickness);
    support.add_method(unit_name, TextApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "outline_color", TextApi::get_outline_color);
    support.add_method(unit_name, TextApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "outline_color=(_)", TextApi::set_outline_color);
    support.add_method(unit_name, TextApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "line_spacing", TextApi::get_line_spacing);
    support.add_method(unit_name, TextApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "line_spacing=(_)", TextApi::set_line_spacing);
    support.add_method(unit_name, TextApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "letter_spacing", TextApi::get_letter_spacing);
    support.add_method(unit_name, TextApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "letter_spacing=(_)", TextApi::set_letter_spacing);
    support.add_method(unit_name, TextApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "paragraph_width", TextApi::get_paragraph_width);
    support.add_method(unit_name, TextApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "paragraph_width=(_)", TextApi::set_paragraph_width);
    support.add_method(unit_name, TextApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "alignment", TextApi::get_alignment);
    support.add_method(unit_name, TextApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "alignment=(_)", TextApi::set_alignment);

    #define X(name) support.add_method(unit_name, AlignmentApi::class_name, AGATE_FOREIGN_METHOD_CLASS, #name, AlignmentApi::name);
    GAMMA_ALIGNMENT_LIST
    #undef X

  }

}
