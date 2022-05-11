#version 330
uniform sampler2D plasma;
uniform sampler1D lave;
uniform float cycle;
in vec2 vsoTexCoord;
in float alpha;

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec4 blurFactor;

void main(void) {
  vec2 t = vec2(2.0, 1.0) * vsoTexCoord;
  if(t.x > 1.0)
    t.x = 2 - t.x;
  fragColor = vec4(texture(lave, texture(plasma, t).r * 3 + cycle).rgb, 1);
  blurFactor = vec4(vec3(alpha), 1.0);
}
