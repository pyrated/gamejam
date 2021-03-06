#version 130

varying vec2 frag_st;
varying vec3 frag_color;
uniform sampler2D tex;

void main() {

  vec4 color = texture(tex, frag_st);
  if (color.a == 0) discard;

  if (color == vec4(1, 1, 1, 1))
    color = vec4(frag_color, 1);

  gl_FragColor = color;
}