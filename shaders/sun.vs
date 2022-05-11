#version 330

uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;
uniform float cycle;

layout (location = 0) in vec3 vsiPosition;
layout (location = 1) in vec3 vsiNormal;
layout (location = 2) in vec2 vsiTexCoord;
 
out vec2 vsoTexCoord;
out vec3 vsoNormal;
out vec4 vsoModPosition;
out vec3 vsoPosition;
out float alpha;

#define M_PI 3.1415926535897932384626433832795

void main(void) {
  float a = 5.0 * 2.0 * M_PI;
  float v = abs(cos(vsiTexCoord.x * a) * sin(vsiTexCoord.y * a)), pv = pow(v, 10);
  float zv = v / (1.0 + pow(abs(vsiPosition.z), 4));
  vec3 p = normalize(cross(vsiPosition, vec3(0,vsiPosition.yz) + vec3(0,0,0.1)));
  vsoNormal = (transpose(inverse(modelViewMatrix)) * vec4(vsiNormal.xyz, 0.0)).xyz;
  vsoPosition = vsiPosition + 3 * vsiPosition * pv + 0.1 * p * sin(62.83 * v + 10 * cycle);
  vsoModPosition = modelViewMatrix * vec4(vsoPosition.xyz, 1.0);
  gl_Position = projectionMatrix * modelViewMatrix * vec4(vsoPosition.xyz, 1.0);
  vsoTexCoord = vsiTexCoord;
  alpha = clamp(1 - pow(zv, 3), 0, 1);
}
