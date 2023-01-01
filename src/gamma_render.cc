#include "gamma_render.h"

#include <cassert>
#include <cmath>
#include <cstdio>

#include "gamma_agate.h"
#include "gamma_debug.h"
#include "gamma_sprite.h"
#include "gamma_text.h"

#include "shaders/default.vert.h"
#include "shaders/default.frag.h"
#include "shaders/default_alpha.frag.h"

namespace gma {

  /*
   * Camera
   */

  void Camera::update(Vec2I framebuffer_size) {
    computed_size = expected_size;
    computed_viewport = expected_viewport;

    switch (type) {
      case CameraType::EXTEND: {
        const float world_ratio = expected_size.x / expected_size.y;

        Vec2F viewport_size = framebuffer_size * expected_viewport.size;
        const float framebuffer_ratio = viewport_size.x / viewport_size.y;

        Vec2F size = expected_size;

        if (framebuffer_ratio < world_ratio) {
          const float ratio = framebuffer_ratio / world_ratio;
          size.y /= ratio;
        } else {
          const float ratio = world_ratio / framebuffer_ratio;
          size.x /= ratio;
        }

        computed_size = size;
        break;
      }
      case CameraType::FILL: {
        const float world_ratio = expected_size.x / expected_size.y;

        Vec2F viewport_size = framebuffer_size * expected_viewport.size;
        const float framebuffer_ratio = viewport_size.x / viewport_size.y;

        Vec2F size = expected_size;

        if (framebuffer_ratio < world_ratio) {
          const float ratio = framebuffer_ratio / world_ratio;
          size.x *= ratio;
        } else {
          float ratio = world_ratio / framebuffer_ratio;
          size.y *= ratio;
        }

        computed_size = size;
        break;
      }
      case CameraType::FIT: {
        const float world_ratio = expected_size.x / expected_size.y;

        Vec2F viewport_size = framebuffer_size * expected_viewport.size;
        const float framebuffer_ratio = viewport_size.x / viewport_size.y;

        RectF viewport;

        if (framebuffer_ratio < world_ratio) {
          const float ratio = framebuffer_ratio / world_ratio;

          viewport.position.x = 0.0f;
          viewport.size.x = 1.0f;

          viewport.position.y = (1.0f - ratio) / 2.0f;
          viewport.size.y = ratio;
        } else {
          const float ratio = world_ratio / framebuffer_ratio;

          viewport.position.y = 0.0f;
          viewport.size.y = 1.0f;

          viewport.position.x = (1.0f - ratio) / 2.0f;
          viewport.size.x = ratio;
        }

        computed_viewport.position = viewport.position * expected_viewport.size + expected_viewport.position;
        computed_viewport.size = viewport.size * expected_viewport.size;
        break;
      }
      case CameraType::LOCKED: {
        Vec2F size = expected_size;
        Vec2F viewport_size = framebuffer_size * expected_viewport.size;

        RectF viewport;

        if (size.x > viewport_size.x) {
          viewport.position.x = 0.0f;
          viewport.size.x = 1.0f;
          size.x = viewport_size.x;
        } else {
          viewport.size.x = size.x / viewport_size.x;
          viewport.position.x = (1.0f - size.x) / 2.0f;
        }

        if (size.y > viewport_size.y) {
          viewport.position.y = 0.0f;
          viewport.size.y = 1.0f;
          size.y = viewport_size.y;
        } else {
          viewport.size.y = size.y / viewport_size.y;
          viewport.position.y = (1.0f - size.y) / 2.0f;
        }

        computed_size = size;
        computed_viewport = viewport;
        break;
      }
      case CameraType::SCREEN: {
        Vec2F viewport_size = framebuffer_size * expected_viewport.size;

        computed_size = viewport_size;
        center = viewport_size / 2.0f;
        break;
      }
      case CameraType::STRETCH:
        /* nothing to do */
        break;
    }
  }

  RectI Camera::compute_viewport(Vec2I framebuffer_size) const {
    RectI viewport;
    viewport.position.x = (int) (framebuffer_size.x * computed_viewport.position.x + 0.5f);
    viewport.position.y = (int) (framebuffer_size.y * computed_viewport.position.y + 0.5f);
    viewport.size.x = (int) (framebuffer_size.x * computed_viewport.size.x + 0.5f);
    viewport.size.y = (int) (framebuffer_size.y * computed_viewport.size.y + 0.5f);
    return viewport;
  }

  Mat3F Camera::compute_view_matrix() const {
    const float sx = 2.0f / computed_size.x;
    const float sy = - 2.0f / computed_size.y;
    const float cos_v = std::cos(rotation);
    const float sin_v = std::sin(rotation);
    const float tx = center.x;
    const float ty = center.y;

    Mat3F mat;
    mat(0, 0) =  sx * cos_v; mat(0, 1) = sx * sin_v; mat(0, 2) = sx * (- tx * cos_v - ty * sin_v);
    mat(1, 0) = -sy * sin_v; mat(1, 1) = sy * cos_v; mat(1, 2) = sy * (  tx * sin_v - ty * cos_v);
    mat(2, 0) = 0.0f;        mat(2, 1) = 0.0f;       mat(2, 2) = 1.0f;
    return mat;
  }


  struct CameraApi : CameraClass {

    static void generic_new(AgateVM *vm, CameraType type) {
      assert(agateCheckTag<CameraClass>(vm, 0));
      auto camera = agateSlotGet<CameraClass>(vm, 0);

      Vec2F center;

      if (!agateCheck(vm, 1, center)) {
        agateError(vm, "Vec2F parameter expected for `center`.");
        return;
      }

      Vec2F size;

      if (!agateCheck(vm, 2, size)) {
        agateError(vm, "Vec2F parameter expected for `size`.");
        return;
      }

      *camera = Camera(type, center, size);
    }

    static void new_extend(AgateVM *vm) { generic_new(vm, CameraType::EXTEND); }
    static void new_fill(AgateVM *vm) { generic_new(vm, CameraType::FILL); }
    static void new_fit(AgateVM *vm) { generic_new(vm, CameraType::FIT); }
    static void new_locked(AgateVM *vm) { generic_new(vm, CameraType::LOCKED); }
    static void new_screen(AgateVM *vm) { generic_new(vm, CameraType::SCREEN); }
    static void new_stretch(AgateVM *vm) { generic_new(vm, CameraType::STRETCH); }

    static void get_size(AgateVM *vm) {
      assert(agateCheckTag<CameraClass>(vm, 0));
      auto camera = agateSlotGet<CameraClass>(vm, 0);

      auto result = agateSlotNew<Vec2FClass>(vm, AGATE_RETURN_SLOT);
      *result = camera->expected_size;
    }

    static void set_size(AgateVM *vm) {
      assert(agateCheckTag<CameraClass>(vm, 0));
      auto camera = agateSlotGet<CameraClass>(vm, 0);

      if (!agateCheck(vm, 1, camera->expected_size)) {
        agateError(vm, "Vec2F parameter expected for `value`.");
        return;
      }

      agateSlotCopy(vm, AGATE_RETURN_SLOT, 1);
    }

    static void get_center(AgateVM *vm) {
      assert(agateCheckTag<CameraClass>(vm, 0));
      auto camera = agateSlotGet<CameraClass>(vm, 0);

      auto result = agateSlotNew<Vec2FClass>(vm, AGATE_RETURN_SLOT);
      *result = camera->center;
    }

    static void set_center(AgateVM *vm) {
      assert(agateCheckTag<CameraClass>(vm, 0));
      auto camera = agateSlotGet<CameraClass>(vm, 0);

      if (!agateCheck(vm, 1, camera->center)) {
        agateError(vm, "Vec2F parameter expected for `value`.");
        return;
      }

      agateSlotCopy(vm, AGATE_RETURN_SLOT, 1);
    }

    static void move1(AgateVM *vm) {
      assert(agateCheckTag<CameraClass>(vm, 0));
      auto camera = agateSlotGet<CameraClass>(vm, 0);

      Vec2F offset;

      if (!agateCheck(vm, 1, offset)) {
        agateError(vm, "Vec2F parameter expected for `offset`.");
        return;
      }

      camera->center += offset;
    }

    static void move2(AgateVM *vm) {
      assert(agateCheckTag<CameraClass>(vm, 0));
      auto camera = agateSlotGet<CameraClass>(vm, 0);

      Vec2F offset;

      if (!agateCheck(vm, 1, offset.x)) {
        agateError(vm, "Float parameter expected for `x`.");
        return;
      }

      if (!agateCheck(vm, 2, offset.y)) {
        agateError(vm, "Float parameter expected for `y`.");
        return;
      }

      camera->center += offset;
    }


    static void get_rotation(AgateVM *vm) {
      assert(agateCheckTag<CameraClass>(vm, 0));
      auto camera = agateSlotGet<CameraClass>(vm, 0);
      agateSlotSetFloat(vm, AGATE_RETURN_SLOT, camera->rotation);
    }

    static void set_rotation(AgateVM *vm) {
      assert(agateCheckTag<CameraClass>(vm, 0));
      auto camera = agateSlotGet<CameraClass>(vm, 0);

      if (!agateCheck(vm, 1, camera->rotation)) {
        agateError(vm, "Float parameter expected for `value`.");
        return;
      }

      agateSlotCopy(vm, AGATE_RETURN_SLOT, 1);
    }

    static void rotate(AgateVM *vm) {
      assert(agateCheckTag<CameraClass>(vm, 0));
      auto camera = agateSlotGet<CameraClass>(vm, 0);

      float angle;

      if (!agateCheck(vm, 1, angle)) {
        agateError(vm, "Float parameter expected for `angle`.");
        return;
      }

      camera->rotation += angle;
    }

    static void zoom1(AgateVM *vm) {
      assert(agateCheckTag<CameraClass>(vm, 0));
      auto camera = agateSlotGet<CameraClass>(vm, 0);

      float factor = 1.0;

      if (!agateCheck(vm, 1, factor)) {
        agateError(vm, "Float parameter expected for `factor`.");
        return;
      }

      camera->expected_size *= factor;
    }

    static void zoom2(AgateVM *vm) {
      assert(agateCheckTag<CameraClass>(vm, 0));
      auto camera = agateSlotGet<CameraClass>(vm, 0);

      float factor = 1.0;

      if (!agateCheck(vm, 1, factor)) {
        agateError(vm, "Float parameter expected for `factor`.");
        return;
      }

      Vec2F fixed;

      if (!agateCheck(vm, 1, fixed)) {
        agateError(vm, "Vec2F parameter expected for `fixed`.");
        return;
      }

      camera->center += (fixed - camera->center) * (1.0f - factor);
      camera->expected_size *= factor;
    }

    static void get_viewport(AgateVM *vm) {
      assert(agateCheckTag<CameraClass>(vm, 0));
      auto camera = agateSlotGet<CameraClass>(vm, 0);

      auto result = agateSlotNew<RectFClass>(vm, AGATE_RETURN_SLOT);
      *result = camera->expected_viewport;
    }

    static void set_viewport(AgateVM *vm) {
      assert(agateCheckTag<CameraClass>(vm, 0));
      auto camera = agateSlotGet<CameraClass>(vm, 0);

      if (!agateCheck(vm, 1, camera->expected_viewport)) {
        agateError(vm, "RectF parameter expected for `value`.");
        return;
      }

      agateSlotCopy(vm, AGATE_RETURN_SLOT, 1);
    }

  };

  /*
   * Transform
   */

  Mat3F Transform::compute_matrix(RectF bounds) const {
    const float ox = origin.x * bounds.size.x + bounds.position.x;
    const float oy = origin.y * bounds.size.y + bounds.position.y;
    const float px = position.x;
    const float py = position.y;
    const float cos_v = std::cos(rotation);
    const float sin_v = std::sin(rotation);
    const float sx = scale.x;
    const float sy = scale.y;

    Mat3F mat;
    mat(0, 0) = sx * cos_v; mat(0, 1) = - sy * sin_v; mat(0, 2) = - ox * mat(0, 0) - oy * mat(0, 1) + px;
    mat(1, 0) = sx * sin_v; mat(1, 1) =   sy * cos_v; mat(1, 2) = - ox * mat(1, 0) - oy * mat(1, 1) + py;
    mat(2, 0) = 0.0f;       mat(2, 1) = 0.0f;         mat(2, 2) = 1.0f;
    return mat;
  }

  struct TransformApi : TransformClass {
    static void new0(AgateVM *vm) {
      assert(agateCheckTag<TransformClass>(vm, 0));
      auto transform = agateSlotGet<TransformClass>(vm, 0);

      transform->origin = vec(0.0f, 0.0f);
      transform->position = vec(0.0f, 0.0f);
      transform->rotation = 0.0f;
      transform->scale = vec(1.0f, 1.0f);
    }

    static void new1(AgateVM *vm) {
      assert(agateCheckTag<TransformClass>(vm, 0));
      auto transform = agateSlotGet<TransformClass>(vm, 0);

      if (!agateCheck(vm, 1, transform->position)) {
        agateError(vm, "Vec2F parameter expected for `position`.");
        return;
      }

      transform->origin = vec(0.0f, 0.0f);
      transform->rotation = 0.0f;
      transform->scale = vec(1.0f, 1.0f);
    }

    static void new2(AgateVM *vm) {
      assert(agateCheckTag<TransformClass>(vm, 0));
      auto transform = agateSlotGet<TransformClass>(vm, 0);

      if (!agateCheck(vm, 1, transform->position)) {
        agateError(vm, "Vec2F parameter expected for `position`.");
        return;
      }

      if (!agateCheck(vm, 2, transform->origin)) {
        agateError(vm, "Vec2F parameter expected for `origin`.");
        return;
      }

      transform->rotation = 0.0f;
      transform->scale = vec(1.0f, 1.0f);
    }

    static void new3(AgateVM *vm) {
      assert(agateCheckTag<TransformClass>(vm, 0));
      auto transform = agateSlotGet<TransformClass>(vm, 0);

      if (!agateCheck(vm, 1, transform->position)) {
        agateError(vm, "Vec2F parameter expected for `position`.");
        return;
      }

      if (!agateCheck(vm, 2, transform->origin)) {
        agateError(vm, "Vec2F parameter expected for `origin`.");
        return;
      }

      if (!agateCheck(vm, 3, transform->rotation)) {
        agateError(vm, "Float parameter expected for `angle`.");
        return;
      }

      transform->scale = vec(1.0f, 1.0f);
    }

    static void get_origin(AgateVM *vm) {
      assert(agateCheckTag<TransformClass>(vm, 0));
      auto transform = agateSlotGet<TransformClass>(vm, 0);

      auto result = agateSlotNew<Vec2FClass>(vm, AGATE_RETURN_SLOT);
      *result = transform->origin;
    }

    static void set_origin(AgateVM *vm) {
      assert(agateCheckTag<TransformClass>(vm, 0));
      auto transform = agateSlotGet<TransformClass>(vm, 0);

      if (!agateCheck(vm, 1, transform->origin)) {
        agateError(vm, "Vec2F parameter expected for `value`.");
        return;
      }

      agateSlotCopy(vm, AGATE_RETURN_SLOT, 1);
    }

    static void get_position(AgateVM *vm) {
      assert(agateCheckTag<TransformClass>(vm, 0));
      auto transform = agateSlotGet<TransformClass>(vm, 0);

      auto result = agateSlotNew<Vec2FClass>(vm, AGATE_RETURN_SLOT);
      *result = transform->position;
    }

    static void set_position(AgateVM *vm) {
      assert(agateCheckTag<TransformClass>(vm, 0));
      auto transform = agateSlotGet<TransformClass>(vm, 0);

      if (!agateCheck(vm, 1, transform->position)) {
        agateError(vm, "Vec2F parameter expected for `value`.");
        return;
      }

      agateSlotCopy(vm, AGATE_RETURN_SLOT, 1);
    }

    static void move(AgateVM *vm) {
      assert(agateCheckTag<TransformClass>(vm, 0));
      auto transform = agateSlotGet<TransformClass>(vm, 0);

      Vec2F offset;

      if (!agateCheck(vm, 1, offset)) {
        agateError(vm, "Vec2F parameter expected for `offset`.");
        return;
      }

      transform->position += offset;
    }

    static void get_rotation(AgateVM *vm) {
      assert(agateCheckTag<TransformClass>(vm, 0));
      auto transform = agateSlotGet<TransformClass>(vm, 0);
      agateSlotSetFloat(vm, AGATE_RETURN_SLOT, transform->rotation);
    }

    static void set_rotation(AgateVM *vm) {
      assert(agateCheckTag<TransformClass>(vm, 0));
      auto transform = agateSlotGet<TransformClass>(vm, 0);

      if (!agateCheck(vm, 1, transform->rotation)) {
        agateError(vm, "Float parameter expected for `value`.");
        return;
      }

      agateSlotCopy(vm, AGATE_RETURN_SLOT, 1);
    }

    static void rotate(AgateVM *vm) {
      assert(agateCheckTag<TransformClass>(vm, 0));
      auto transform = agateSlotGet<TransformClass>(vm, 0);

      float angle;

      if (!agateCheck(vm, 1, angle)) {
        agateError(vm, "Float parameter expected for `angle`.");
        return;
      }

      transform->rotation += angle;
    }

    static void get_scale(AgateVM *vm) {
      assert(agateCheckTag<TransformClass>(vm, 0));
      auto transform = agateSlotGet<TransformClass>(vm, 0);

      auto result = agateSlotNew<Vec2FClass>(vm, AGATE_RETURN_SLOT);
      *result = transform->scale;
    }

    static void set_scale(AgateVM *vm) {
      assert(agateCheckTag<TransformClass>(vm, 0));
      auto transform = agateSlotGet<TransformClass>(vm, 0);

      if (!agateCheck(vm, 1, transform->scale)) {
        agateError(vm, "Vec2F parameter expected for `value`.");
        return;
      }

      agateSlotCopy(vm, AGATE_RETURN_SLOT, 1);
    }

    static void scale(AgateVM *vm) {
      assert(agateCheckTag<TransformClass>(vm, 0));
      auto transform = agateSlotGet<TransformClass>(vm, 0);

      Vec2F factor;

      if (!agateCheck(vm, 1, factor)) {
        agateError(vm, "Vec2F parameter expected for `factor`.");
        return;
      }

      transform->scale *= factor;
    }

  };

  /*
   * Shader
   */

  #define GAMMA_INFO_LOG_MAX 1024

  static GLuint shader_compile(const char *source, GLenum type) {
    GLuint shader = glCreateShader(type);
    GAMMA_GL_CHECK_HERE();

    if (shader == 0) {
      return 0;
    }

    GAMMA_GL_CHECK(glShaderSource(shader, 1, &source, nullptr));
    GAMMA_GL_CHECK(glCompileShader(shader));

    GLint status = GL_FALSE;
    GAMMA_GL_CHECK(glGetShaderiv(shader, GL_COMPILE_STATUS, &status));

    if (status != GL_TRUE) {
      char info_log[GAMMA_INFO_LOG_MAX];
      GAMMA_GL_CHECK(glGetShaderInfoLog(shader, GAMMA_INFO_LOG_MAX, nullptr, info_log));
      std::fprintf(stderr, "%s\n", info_log);
      return 0;
    }

    return shader;
  }

  static GLuint shader_compile_program(const char *vertex_source, const char *fragment_source) {
    GLuint program = glCreateProgram();

    if (vertex_source != nullptr) {
      GLuint shader = shader_compile(vertex_source, GL_VERTEX_SHADER);
      GAMMA_GL_CHECK(glAttachShader(program, shader));
      GAMMA_GL_CHECK(glDeleteShader(shader)); // the shader is still here because it is attached to the program
    }

    if (fragment_source != nullptr) {
      GLuint shader = shader_compile(fragment_source, GL_FRAGMENT_SHADER);
      GAMMA_GL_CHECK(glAttachShader(program, shader));
      GAMMA_GL_CHECK(glDeleteShader(shader)); // the shader is still here because it is attached to the program
    }

    GAMMA_GL_CHECK(glLinkProgram(program));

    GLint status = GL_FALSE;
    GAMMA_GL_CHECK(glGetProgramiv(program, GL_LINK_STATUS, &status));

    if (status != GL_TRUE) {
      char info_log[GAMMA_INFO_LOG_MAX];
      GAMMA_GL_CHECK(glGetProgramInfoLog(program, GAMMA_INFO_LOG_MAX, nullptr, info_log));
      std::fprintf(stderr, "%s\n", info_log);
      return 0;
    }

    return program;
  }


  /*
   * Renderer
   */

  Renderer::Renderer(AgateVM *vm, Window *window) {
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

    assert(window->ptr != nullptr);
    context = SDL_GL_CreateContext(window->ptr);

    if (context == nullptr) {
      agateError(vm, "Unable to create a context: %s\n", SDL_GetError());
      return;
    }

    if (SDL_GL_MakeCurrent(window->ptr, context) != 0) {
      agateError(vm, "Unable to make the context current: %s\n", SDL_GetError());
      return;
    }

    if (gladLoadGLLoader(SDL_GL_GetProcAddress) == 0) {
      agateError(vm, "Unable to load OpenGL 3.3.\n");
      return;
    }

    GAMMA_GL_CHECK(glEnable(GL_BLEND));
    GAMMA_GL_CHECK(glEnable(GL_SCISSOR_TEST));

    GAMMA_GL_CHECK(glGenVertexArrays(1, &vao));
    GAMMA_GL_CHECK(glBindVertexArray(vao));

    default_shader = shader_compile_program(gamma_default_vert, gamma_default_frag);
    default_alpha_shader = shader_compile_program(gamma_default_vert, gamma_default_alpha_frag);

    const uint8_t pixel[] = { 0xFF, 0xFF, 0xFF, 0xFF };

    Texture texture(TextureKind::COLOR, 1, 1, pixel);
    default_texture = texture.id;
    assert(default_texture != 0);

    GAMMA_GL_CHECK(glBindTexture(GL_TEXTURE_2D, default_texture));
    GAMMA_GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
    GAMMA_GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));
    GAMMA_GL_CHECK(glBindTexture(GL_TEXTURE_2D, 0));

    SDL_GL_GetDrawableSize(window->ptr, &framebuffer_size.x, &framebuffer_size.y);

    camera = Camera(CameraType::SCREEN, vec(0.0f, 0.0f), vec(1.0f, 1.0f));
    camera.update(framebuffer_size);
  }

  void Renderer::destroy() {
    if (context == nullptr) {
      return;
    }

    GAMMA_GL_CHECK(glBindTexture(GL_TEXTURE_2D, 0));

    if (default_texture != 0) {
      GAMMA_GL_CHECK(glDeleteTextures(1, &default_texture));
      default_texture = 0;
    }

    GAMMA_GL_CHECK(glUseProgram(0));

    if (default_alpha_shader != 0) {
      GAMMA_GL_CHECK(glDeleteProgram(default_alpha_shader));
      default_alpha_shader = 0;
    }

    if (default_shader != 0) {
      GAMMA_GL_CHECK(glDeleteProgram(default_shader));
      default_shader = 0;
    }

    GAMMA_GL_CHECK(glBindVertexArray(0));
    GAMMA_GL_CHECK(glDeleteVertexArrays(1, &vao));
    vao = 0;

    SDL_GL_DeleteContext(context);
    context = nullptr;
  }

  void Renderer::draw(const RendererData& submitted_data) {
    if (submitted_data.vertex_buffer == 0) {
      return;
    }

    RendererData data = submitted_data;

    // textures

    if (data.texture0 == 0) {
      data.texture0 = default_texture;
    }

    if (data.texture1 == 0) {
      data.texture1 = default_texture;
    }

    // shader

    if (data.shader == 0) {
      if (data.mode == RendererMode::ALPHA) {
        data.shader = default_alpha_shader;
      } else {
        assert(data.mode == RendererMode::COLOR);
        data.shader = default_shader;
      }
    }

    GAMMA_GL_CHECK(glUseProgram(data.shader));
    GLint index = 0;

    GLint texture0_location = glGetUniformLocation(data.shader, "texture0");
    GAMMA_GL_CHECK_HERE();

    if (texture0_location != -1) {
      GAMMA_GL_CHECK(glActiveTexture(GL_TEXTURE0 + index));
      GAMMA_GL_CHECK(glBindTexture(GL_TEXTURE_2D, data.texture0));
      GAMMA_GL_CHECK(glUniform1i(texture0_location, index++));
    }

    GLint texture1_location = glGetUniformLocation(data.shader, "texture1");
    GAMMA_GL_CHECK_HERE();

    if (texture1_location != -1) {
      GAMMA_GL_CHECK(glActiveTexture(GL_TEXTURE0 + index));
      GAMMA_GL_CHECK(glBindTexture(GL_TEXTURE_2D, data.texture1));
      GAMMA_GL_CHECK(glUniform1i(texture1_location, index++));
    }

    // transform

    Mat3F view = camera.compute_view_matrix();
    Mat3F transform = view * data.transform;

    GLint transform_location = glGetUniformLocation(data.shader, "transform");
    GAMMA_GL_CHECK_HERE();

    if (transform_location != -1) {
      GAMMA_GL_CHECK(glUniformMatrix3fv(transform_location, 1, GL_FALSE, transform.data()));
    }

    // blend

    GAMMA_GL_CHECK(glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD));
    GAMMA_GL_CHECK(glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA));

    // buffers

    GAMMA_GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, data.vertex_buffer));

    if (data.element_buffer != 0) {
      GAMMA_GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, data.element_buffer));
    }

    // inputs

    GAMMA_GL_CHECK(glEnableVertexAttribArray(0));
    GAMMA_GL_CHECK(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *) offsetof(Vertex, position)));

    GAMMA_GL_CHECK(glEnableVertexAttribArray(1));
    GAMMA_GL_CHECK(glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *) offsetof(Vertex, color)));

    GAMMA_GL_CHECK(glEnableVertexAttribArray(2));
    GAMMA_GL_CHECK(glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *) offsetof(Vertex, texcoords)));

    // draw

    if (data.element_buffer == 0) {
      GAMMA_GL_CHECK(glDrawArrays(data.primitive, 0, data.count));
    } else {
      GAMMA_GL_CHECK(glDrawElements(data.primitive, data.count, GL_UNSIGNED_SHORT, nullptr));
    }

    // cleanup

    GAMMA_GL_CHECK(glDisableVertexAttribArray(2));
    GAMMA_GL_CHECK(glDisableVertexAttribArray(1));
    GAMMA_GL_CHECK(glDisableVertexAttribArray(0));

    GAMMA_GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, 0));

    if (data.element_buffer != 0) {
      GAMMA_GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
    }
  }

  Vec2I Renderer::world_to_device(Vec2F position, const Camera *camera_ptr) {
    RectI viewport = camera_ptr->compute_viewport(framebuffer_size);

    /* apply view transform
    * i.e. compute normalized device coordinates from world coordinates
    */

    Mat3F view = camera_ptr->compute_view_matrix();
    Vec2F normalized = transform_point(view, position);

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

    Vec2I result;
    result.x = (int) (1 + normalized.x / 2 * viewport.size.x + viewport.position.x);
    result.y = (int) (1 - normalized.y / 2 * viewport.size.y + viewport.position.y);
    return result;
  }

  Vec2F Renderer::device_to_world(Vec2I coordinates, const Camera *camera_ptr) {
    RectI viewport = camera_ptr->compute_viewport(framebuffer_size);

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

    Vec2F normalized;
    normalized.x = 2.0f * (coordinates.x - viewport.position.x) / viewport.size.x - 1.0f;
    normalized.y = 1.0f - 2.0f * (coordinates.y - viewport.position.y) / viewport.size.y;

    /* apply inverse view transform
    * i.e. compute world coordinates from normalized device coordinates
    */

    Mat3F view = camera_ptr->compute_view_matrix();
    Mat3F inverse_view = inverse(view);
    return transform_point(inverse_view, normalized);
  }


  struct RendererApi : RendererClass {
    static void destroy(AgateVM *vm, const char *unit_name, const char *class_name, void *data) {
      auto renderer = static_cast<Renderer *>(data);
      renderer->destroy();
    }


    static void new1(AgateVM *vm) {
      assert(agateCheckTag<RendererClass>(vm, 0));
      auto renderer = agateSlotGet<RendererClass>(vm, 0);

      if (!agateCheckTag<WindowClass>(vm, 1)) {
        agateError(vm, "Window parameter expected for `window`.");
        return;
      }

      auto window = agateSlotGet<WindowClass>(vm, 1);
      *renderer = Renderer(vm, window);
    }

    static void clear0(AgateVM *vm) {
      assert(agateCheckTag<RendererClass>(vm, 0));
      auto renderer = agateSlotGet<RendererClass>(vm, 0);

      GAMMA_GL_CHECK(glScissor(0, 0, renderer->framebuffer_size.x, renderer->framebuffer_size.y));
      GAMMA_GL_CHECK(glClear(GL_COLOR_BUFFER_BIT));
    }

    static void clear1(AgateVM *vm) {
      assert(agateCheckTag<RendererClass>(vm, 0));
      auto renderer = agateSlotGet<RendererClass>(vm, 0);

      Color color;

      if (!agateCheck(vm, 1, color)) {
        agateError(vm, "Color parameter expected for `color`.");
        return;
      }

      GAMMA_GL_CHECK(glClearColor(color.r, color.g, color.b, color.a));
      GAMMA_GL_CHECK(glScissor(0, 0, renderer->framebuffer_size.x, renderer->framebuffer_size.y));
      GAMMA_GL_CHECK(glClear(GL_COLOR_BUFFER_BIT));
    }

    static void display(AgateVM *vm) {
      SDL_GL_SwapWindow(SDL_GL_GetCurrentWindow());
    }

    static void set_camera(AgateVM *vm) {
      assert(agateCheckTag<RendererClass>(vm, 0));
      auto renderer = agateSlotGet<RendererClass>(vm, 0);

      if (!agateCheckTag<CameraClass>(vm, 1)) {
        agateError(vm, "Camera parameter expected for `value`.");
        return;
      }

      const auto camera = agateSlotGet<CameraClass>(vm, 1);

      renderer->camera = *camera;

      SDL_GL_GetDrawableSize(SDL_GL_GetCurrentWindow(), &renderer->framebuffer_size.x, &renderer->framebuffer_size.y);
      renderer->camera.update(renderer->framebuffer_size);

      RectI viewport = renderer->camera.compute_viewport(renderer->framebuffer_size);
      viewport.position.y = renderer->framebuffer_size.y - (viewport.position.y + viewport.size.y); // invert y

      // set the viewport everytime a new camera is defined
      GAMMA_GL_CHECK(glViewport(viewport.position.x, viewport.position.y, viewport.size.x, viewport.size.y));

      // the viewport does not scissor
      GAMMA_GL_CHECK(glScissor(viewport.position.x, viewport.position.y, viewport.size.x, viewport.size.y));
    }

    static void world_to_device1(AgateVM *vm) {
      assert(agateCheckTag<RendererClass>(vm, 0));
      auto renderer = agateSlotGet<RendererClass>(vm, 0);

      Vec2F position;

      if (!agateCheck(vm, 1, position)) {
        agateError(vm, "Vec2F parameter expected for `position`.");
        return;
      }

      auto result = agateSlotNew<Vec2IClass>(vm, AGATE_RETURN_SLOT);
      *result = renderer->world_to_device(position, &renderer->camera);
    }

    static void world_to_device2(AgateVM *vm) {
      assert(agateCheckTag<RendererClass>(vm, 0));
      auto renderer = agateSlotGet<RendererClass>(vm, 0);

      Vec2F position;

      if (!agateCheck(vm, 1, position)) {
        agateError(vm, "Vec2F parameter expected for `position`.");
        return;
      }

      if (!agateCheckTag<CameraClass>(vm, 2)) {
        agateError(vm, "Camera parameter expected for `camera`.");
        return;
      }

      auto camera = agateSlotGet<CameraClass>(vm, 2);
      camera->update(renderer->framebuffer_size);

      auto result = agateSlotNew<Vec2IClass>(vm, AGATE_RETURN_SLOT);
      *result = renderer->world_to_device(position, camera);
    }

    static void device_to_world1(AgateVM *vm) {
      assert(agateCheckTag<RendererClass>(vm, 0));
      auto renderer = agateSlotGet<RendererClass>(vm, 0);

      Vec2I coordinates;

      if (!agateCheck(vm, 1, coordinates)) {
        agateError(vm, "Vec2I parameter expected for `coordinates`.");
        return;
      }

      auto result = agateSlotNew<Vec2FClass>(vm, AGATE_RETURN_SLOT);
      *result = renderer->device_to_world(coordinates, &renderer->camera);
    }

    static void device_to_world2(AgateVM *vm) {
      assert(agateCheckTag<RendererClass>(vm, 0));
      auto renderer = agateSlotGet<RendererClass>(vm, 0);

      Vec2I coordinates;

      if (!agateCheck(vm, 1, coordinates)) {
        agateError(vm, "Vec2I parameter expected for `coordinates`.");
        return;
      }

      if (!agateCheckTag<CameraClass>(vm, 2)) {
        agateError(vm, "Camera parameter expected for `camera`.");
        return;
      }

      auto camera = agateSlotGet<CameraClass>(vm, 2);
      camera->update(renderer->framebuffer_size);

      auto result = agateSlotNew<Vec2FClass>(vm, AGATE_RETURN_SLOT);
      *result = renderer->device_to_world(coordinates, camera);
    }

    static void draw_object(AgateVM *vm) {
      assert(agateCheckTag<RendererClass>(vm, 0));
      auto renderer = agateSlotGet<RendererClass>(vm, 0);

      if (agateSlotType(vm, 1) != AGATE_TYPE_FOREIGN) {
        agateError(vm, "Graphical object parameter expected for `object`.");
        return;
      }

      if (!agateCheckTag<TransformClass>(vm, 2)) {
        agateError(vm, "Transform parameter expected for `transform`.");
        return;
      }

      const auto transform = agateSlotGet<TransformClass>(vm, 2);

      switch (agateSlotGetForeignTag(vm, 1)) {
        case SpriteClass::tag: {
          auto sprite = agateSlotGet<SpriteClass>(vm, 1);
          sprite->render(*renderer, *transform);
          break;
        }

        case TextClass::tag: {
          auto text = agateSlotGet<TextClass>(vm, 1);
          text->render(*renderer, *transform);
          break;
        }

        default:
          agateError(vm, "Graphical object parameter expected for `object`.");
          break;
      }
    }

    static void draw_rect2(AgateVM *vm) {
      assert(agateCheckTag<RendererClass>(vm, 0));
      auto renderer = agateSlotGet<RendererClass>(vm, 0);

      RectF rect;

      if (!agateCheck(vm, 1, rect)) {
        agateError(vm, "RectF parameter expected for `rect`.");
        return;
      }

      Color color;

      if (!agateCheck(vm, 2, color)) {
        agateError(vm, "Color parameter expected for `color`.");
        return;
      }

      Vertex vertices[] = {
        { vec(0.0f,         0.0f       ), color, vec(0.0f, 0.0f) },
        { vec(0.0f,         rect.size.y), color, vec(0.0f, 0.0f) },
        { vec(rect.size.x,  0.0f       ), color, vec(0.0f, 0.0f) },
        { vec(rect.size.x,  rect.size.y), color, vec(0.0f, 0.0f) },
      };

      RendererData data;

      data.primitive = GL_TRIANGLE_STRIP;
      data.count = 4;

      GAMMA_GL_CHECK(glGenBuffers(1, &data.vertex_buffer));
      GAMMA_GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, data.vertex_buffer));
      GAMMA_GL_CHECK(glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW));
      GAMMA_GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, 0));

      data.element_buffer = 0;
      data.mode = RendererMode::COLOR;
      data.texture0 = 0;
      data.texture1 = 0;
      data.shader = 0;
      data.transform = translation(rect.position);

      renderer->draw(data);

      GAMMA_GL_CHECK(glDeleteBuffers(1, &data.vertex_buffer));
    }


    static void is_vsynced(AgateVM *vm) {
      agateSlotSetBool(vm, AGATE_RETURN_SLOT, SDL_GL_GetSwapInterval() != 0);
    }

    static void set_vsynced(AgateVM *vm) {
      bool vsynced;

      if (!agateCheck(vm, 1, vsynced)) {
        agateError(vm, "Bool parameter expected for `value`.");
        return;
      }

      if (SDL_GL_SetSwapInterval(vsynced ? 1 : 0) != 0) {
        if (vsynced) {
          agateError(vm, "Unable to set vertical synchronization.");
        } else {
          agateError(vm, "Unable to unset vertical synchronization.");
        }

        return;
      }

      agateSlotCopy(vm, AGATE_RETURN_SLOT, 1);
    }


  };


  /*
   * RenderUnit
   */

  void RenderUnit::provide_support(Support & support) {
    support.add_class_handler(unit_name, CameraClass::class_name, generic_simple_handler<CameraClass>());
    support.add_class_handler(unit_name, TransformClass::class_name, generic_simple_handler<TransformClass>());
    support.add_class_handler(unit_name, RendererClass::class_name, generic_handler<RendererClass>(RendererApi::destroy));

    support.add_method(unit_name, CameraApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "init new_extend(_,_)", CameraApi::new_extend);
    support.add_method(unit_name, CameraApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "init new_fill(_,_)", CameraApi::new_fill);
    support.add_method(unit_name, CameraApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "init new_fit(_,_)", CameraApi::new_fit);
    support.add_method(unit_name, CameraApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "init new_locked(_,_)", CameraApi::new_locked);
    support.add_method(unit_name, CameraApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "init new_screen(_,_)", CameraApi::new_screen);
    support.add_method(unit_name, CameraApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "init new_stretch(_,_)", CameraApi::new_stretch);
    support.add_method(unit_name, CameraApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "size", CameraApi::get_size);
    support.add_method(unit_name, CameraApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "size=(_)", CameraApi::set_size);
    support.add_method(unit_name, CameraApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "center", CameraApi::get_center);
    support.add_method(unit_name, CameraApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "center=(_)", CameraApi::set_center);
    support.add_method(unit_name, CameraApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "move(_)", CameraApi::move1);
    support.add_method(unit_name, CameraApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "move(_,_)", CameraApi::move2);
    support.add_method(unit_name, CameraApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "rotation", CameraApi::get_rotation);
    support.add_method(unit_name, CameraApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "rotation=(_)", CameraApi::set_rotation);
    support.add_method(unit_name, CameraApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "rotate(_)", CameraApi::rotate);
    support.add_method(unit_name, CameraApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "zoom(_)", CameraApi::zoom1);
    support.add_method(unit_name, CameraApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "zoom(_,_)", CameraApi::zoom2);
    support.add_method(unit_name, CameraApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "viewport", CameraApi::get_viewport);
    support.add_method(unit_name, CameraApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "viewport=(_)", CameraApi::set_viewport);

    support.add_method(unit_name, TransformApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "init new()", TransformApi::new0);
    support.add_method(unit_name, TransformApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "init new(_)", TransformApi::new1);
    support.add_method(unit_name, TransformApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "init new(_,_)", TransformApi::new2);
    support.add_method(unit_name, TransformApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "init new(_,_,_)", TransformApi::new3);
    support.add_method(unit_name, TransformApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "origin", TransformApi::get_origin);
    support.add_method(unit_name, TransformApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "origin=(_)", TransformApi::set_origin);
    support.add_method(unit_name, TransformApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "position", TransformApi::get_position);
    support.add_method(unit_name, TransformApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "position=(_)", TransformApi::set_position);
    support.add_method(unit_name, TransformApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "move(_)", TransformApi::move);
    support.add_method(unit_name, TransformApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "rotation", TransformApi::get_rotation);
    support.add_method(unit_name, TransformApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "rotation=(_)", TransformApi::set_rotation);
    support.add_method(unit_name, TransformApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "rotate(_)", TransformApi::rotate);
    support.add_method(unit_name, TransformApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "scale", TransformApi::get_scale);
    support.add_method(unit_name, TransformApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "scale=(_)", TransformApi::set_scale);
    support.add_method(unit_name, TransformApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "scale(_)", TransformApi::scale);

    support.add_method(unit_name, RendererApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "init new(_)", RendererApi::new1);
    support.add_method(unit_name, RendererApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "clear()", RendererApi::clear0);
    support.add_method(unit_name, RendererApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "clear(_)", RendererApi::clear1);
    support.add_method(unit_name, RendererApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "display()", RendererApi::display);
    support.add_method(unit_name, RendererApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "camera=(_)", RendererApi::set_camera);
    support.add_method(unit_name, RendererApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "world_to_device(_)", RendererApi::world_to_device1);
    support.add_method(unit_name, RendererApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "world_to_device(_,_)", RendererApi::world_to_device2);
    support.add_method(unit_name, RendererApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "device_to_world(_)", RendererApi::device_to_world1);
    support.add_method(unit_name, RendererApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "device_to_world(_,_)", RendererApi::device_to_world2);
    support.add_method(unit_name, RendererApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "draw_object(_,_)", RendererApi::draw_object);
    support.add_method(unit_name, RendererApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "draw_rect(_,_)", RendererApi::draw_rect2);
    support.add_method(unit_name, RendererApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "vsynced", RendererApi::is_vsynced);
    support.add_method(unit_name, RendererApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "vsynced=(_)", RendererApi::set_vsynced);
  }

}
