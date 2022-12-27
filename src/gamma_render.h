#ifndef GAMMA_RENDER_H
#define GAMMA_RENDER_H

#include <SDL2/SDL.h>
#include "glad/glad.h"

#include "gamma_color.h"
#include "gamma_support.h"
#include "gamma_math.h"
#include "gamma_window.h"

namespace gma {

  struct RenderUnit {
    static constexpr const char * unit_name = "gamma/render";
    static void provide_support(Support & support);
  };

  /*
   * Camera
   */

  enum class CameraType {
    EXTEND,
    FILL,
    FIT,
    LOCKED,
    SCREEN,
    STRETCH,
  };

  struct Camera {
    CameraType type;
    Vec2F center;
    Vec2F expected_size;
    Vec2F computed_size;
    float rotation;
    RectF expected_viewport;
    RectF computed_viewport;

    Camera() = default;

    Camera(CameraType type, Vec2F center, Vec2F size)
    : type(type)
    , center(center)
    , expected_size(size)
    , computed_size(size)
    , rotation(0.0f)
    , expected_viewport({ { 0.0f, 0.0f }, { 1.0f, 1.0f }})
    , computed_viewport({ { 0.0f, 0.0f }, { 1.0f, 1.0f }})
    {
    }

    void update(Vec2I framebuffer_size);
    RectI compute_viewport(Vec2I framebuffer_size) const;
    Mat3F compute_view_matrix() const;
  };

  struct CameraClass : RenderUnit {
    using type = Camera;
    static constexpr const char * class_name = "Camera";
    static constexpr uint64_t tag = compute_tag(unit_name, class_name);
  };

  /*
   * Transform
   */

  struct Transform {
    Vec2F origin;
    Vec2F position;
    float rotation;
    Vec2F scale;

    Mat3F compute_matrix(RectF bounds) const;
  };

  struct TransformClass : RenderUnit {
    using type = Transform;
    static constexpr const char * class_name = "Transform";
    static constexpr uint64_t tag = compute_tag(unit_name, class_name);
  };

  /*
   * Renderer
   */

  enum class RendererMode {
    COLOR,
    ALPHA,
  };

  struct Vertex {
    Vec2F position;
    Color color;
    Vec2F texcoords;
  };

  struct RendererData {
    GLenum primitive;
    GLsizei count;
    GLuint vertex_buffer;
    GLuint element_buffer;
    RendererMode mode;
    GLuint texture0;
    GLuint texture1;
    GLuint shader;
    RectF bounds;
    Mat3F transform;
  };

  struct Renderer {
    SDL_GLContext context;
    GLuint vao;
    GLuint default_shader;
    GLuint default_alpha_shader;
    GLuint default_texture;

    Vec2I framebuffer_size;
    Camera camera;

    Renderer() = default;
    Renderer(AgateVM *vm, Window *window);

    void destroy();

    void draw(const RendererData& submitted_data);
    Vec2I world_to_device(Vec2F position, const Camera *camera);
    Vec2F device_to_world(Vec2I coordinates, const Camera *camera);
  };

  struct RendererClass : RenderUnit {
    using type = Renderer;
    static constexpr const char * class_name = "Renderer";
    static constexpr uint64_t tag = compute_tag(unit_name, class_name);
  };

}

#endif // GAMMA_RENDER_H
