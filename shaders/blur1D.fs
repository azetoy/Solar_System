/*!\file blur1D.fs
 *
 * \brief flou gaussien uniforme ou pondéré par une weight map (texture).
 *
 * \author Farès BELHADJ amsi@ai.univ-paris8.fr
 * \date october 06, 2015
*/
#version 330

uniform sampler2D myTexture;
uniform sampler2D myWeights;
uniform int nweights, useWeightMap;
uniform float weight[128];
uniform vec2 offset[128];

in  vec2 vsoTexCoord;
out vec4 fragColor;

vec4 uniformBlur(void) {
  vec4 c = texture(myTexture, vsoTexCoord.st) * weight[0];
  for (int i = 1; i < nweights; i++) {
   // c += texture(myTexture, vsoTexCoord.st + offset[i]) * weight[i];
   // c += texture(myTexture, vsoTexCoord.st - offset[i]) * weight[i];

    c += 2.1 * texture(myTexture, vsoTexCoord.st + offset[i]) * weight[i];
  }
  return c;
}

vec4 weightedBlur(void) {
  float w;
  int sub_nweights = 1 + int(float(nweights) * texture(myWeights, vsoTexCoord.st).r);
  vec4 c = texture(myTexture, vsoTexCoord.st) * (w = weight[0]);
  for (int i = 1; i < sub_nweights; i++) {
    w += 2.0 * weight[i];
  }
  for (int i = 1; i < sub_nweights; i++) {
    c += texture(myTexture, vsoTexCoord.st + offset[i]) * weight[i];
    c += texture(myTexture, vsoTexCoord.st - offset[i]) * weight[i];
  }
  return c / w;
}

void main(void) {
  fragColor = (useWeightMap != 0) ? weightedBlur() : uniformBlur();
}
