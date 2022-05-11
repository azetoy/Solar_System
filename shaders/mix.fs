#version 330
uniform sampler2D tex0;
uniform sampler2D tex1;
uniform sampler2D alpha;
uniform int bg;
uniform int sob;
uniform float width, height;
in  vec2 vsoTexCoord;
out vec4 fragColor;


const vec2 G[9] = vec2[]( vec2(1.0,  1.0), vec2(0.0,  2.0), vec2(-1.0,  1.0), 
                          vec2(2.0,  0.0), vec2(0.0,  0.0), vec2(-2.0,  0.0), 
                          vec2(1.0, -1.0), vec2(0.0, -2.0), vec2(-1.0, -1.0) );
vec2 pas = vec2(1.0 / float(width - 1), 1.0 / float(height - 1));
vec2 offset[9] = vec2[](vec2(-pas.x , -pas.y), vec2( 0.0, -pas.y), vec2( pas.x , -pas.y), 
                        vec2(-pas.x, 0.0),       vec2( 0.0, 0.0),      vec2( pas.x, 0.0), 
                        vec2(-pas.x,   pas.y), vec2( 0.0, pas.y),  vec2( pas.x ,  pas.y));

vec4 color(vec2 tc) {
  if(bg == 0)
    return mix( texture(tex0, tc), vec4(0), 1 - texture(alpha, tc).r);
  return mix( texture(tex0, tc), texture(tex1, vec2(tc.x, 1 - tc.y)), 1 - texture(alpha, tc).r);
}

float sobel(void) {
  vec2 g = vec2(0.0, 0.0);
  for(int i = 0; i < 9; i++)
    g += dot(vec3(0.299, 0.587, 0.114), color(vsoTexCoord.st + offset[i]).rgb) * G[i];
  return 1.0 - length(g);
}

void main(void) {
  if(sob != 0)
    fragColor = sobel() * color(vsoTexCoord.st);
  else
    fragColor = color(vsoTexCoord.st);
}


