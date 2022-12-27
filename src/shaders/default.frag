#version 330

in vec4 fragment_color;
in vec2 fragment_tex_coords;

out vec4 output_color;

uniform sampler2D texture0;

void main(void) {
  vec4 color = texture2D(texture0, fragment_tex_coords);
  output_color = color * fragment_color;
}
