#version 330

layout(location = 0) in vec2 vertex_position;
layout(location = 1) in vec4 vertex_color;
layout(location = 2) in vec2 vertex_tex_coords;

out vec4 fragment_color;
out vec2 fragment_tex_coords;

uniform mat3 transform;

void main(void) {
  fragment_color = vertex_color;
  fragment_tex_coords = vertex_tex_coords;

  vec3 world_position = vec3(vertex_position, 1);
  vec3 normalized_position = transform * world_position;

  gl_Position = vec4(normalized_position.xy, 0, 1);
}
