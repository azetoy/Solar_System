#include <blur.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <GL4D/gl4du.h>

#define MAX_RADIUS 128

static GLuint blurPId = 0, width = 1, height = 1;
static GLfloat offsetV[MAX_RADIUS << 1], offsetH[MAX_RADIUS << 1];

/* A utiliser dans le cas de la regénération des poids */
/* static GLfloat weights[(MAX_RADIUS * (MAX_RADIUS + 1)) >> 1]; */

#include <blur_weights.h>

void blurInit(GLuint w, GLuint h) {
  int i;
  width  = w; 
  height = h;
  for(i = 0; i < MAX_RADIUS; i++) {
    offsetH[(i << 1) + 0] = i / (GLfloat)width;
    offsetV[(i << 1) + 1] = i / (GLfloat)height;
    offsetH[(i << 1) + 1] = offsetV[(i << 1) + 0] = 0.0;
  }
  /* A utiliser dans le cas de la regénération des poids */
  /*
    GLfloat d, si = 2., som = 0;
    int i, n, id;
    for(n = 1, id = 0; n <= MAX_RADIUS; id += n, n++) {
      for(i = 0, som = 0.0; i < n; i++) {
	d = 4.0 * i / (GLfloat)n;
	weights[id + i] = (1.0 / (si * sqrt(2.0 * M_PI))) * exp(-d * d / (2 * si * si));
	som += (i ? 2 : 1) * weights[id + i];
      }
      for(i = 0; i < n; i++) {
	weights[id + i] /= som;
	fprintf(stdout, "%f, ", weights[id + i]);
      }
      fprintf(stdout, "\n");
      } */
  #ifdef GLSL4
  blurPId = gl4duCreateProgram("<vs>shaders/blur.vs", "<fs>shaders/blur1D_4.fs", NULL);
  #else
  blurPId = gl4duCreateProgram("<vs>shaders/blur.vs", "<fs>shaders/blur1D.fs", NULL);
  #endif
}

void blur(GLuint plate_vao, GLuint radius, GLuint in, GLuint out, GLuint weight, GLuint nb_iterations, int flipV) {
  int i, n;
  GLuint temp, rin = in;

#ifdef GLSL4 /* je trouve cette méthode lourdingue ! */
  GLint locOfBlurFunc = glGetSubroutineUniformLocation(blurPId, GL_FRAGMENT_SHADER, "blurFunc");
  GLuint bfFunc1 = glGetSubroutineIndex(blurPId, GL_FRAGMENT_SHADER, "uniformBlur");
  GLuint bfFunc2 = glGetSubroutineIndex(blurPId, GL_FRAGMENT_SHADER, "weightedBlur");
  GLsizei nActiveUniforms;
  GLuint * idx;
  glGetProgramStageiv(blurPId, GL_FRAGMENT_SHADER,
		      GL_ACTIVE_SUBROUTINE_UNIFORM_LOCATIONS, &nActiveUniforms);
  idx = calloc(nActiveUniforms, sizeof *idx);
  assert(idx);
#endif

  glGenTextures(1, &temp);
  glBindTexture(GL_TEXTURE_2D, temp);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
  radius = radius > MAX_RADIUS ? MAX_RADIUS : radius;
  for(n = 0; n < (int)nb_iterations; n++) {
    for(i = 0; i < 2; i++) {
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, i == 0 ? temp : out,  0);
      glUseProgram(blurPId);

#ifdef GLSL4
      idx[locOfBlurFunc] = weight ? bfFunc2 : bfFunc1;
      glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, nActiveUniforms, idx);
#endif

      glUniform1i(glGetUniformLocation(blurPId,  "myTexture"), 0);
      glUniform1i(glGetUniformLocation(blurPId,  "myWeights"), 1);
      glUniform1i(glGetUniformLocation(blurPId,  "useWeightMap"), weight ? 1 : 0);
      glUniform1i(glGetUniformLocation(blurPId,  "inv"), i ? flipV : 0);
      glUniform1fv(glGetUniformLocation(blurPId, "weight"), MAX_RADIUS, &weights[(radius * (radius - 1)) >> 1]);
      glUniform2fv(glGetUniformLocation(blurPId, "offset"), MAX_RADIUS, (i % 2) ? offsetH : offsetV);
      glUniform1i(glGetUniformLocation(blurPId,  "nweights"), radius);
      glDisable(GL_DEPTH_TEST);
      glBindVertexArray(plate_vao);
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, i == 0 ? rin : temp);
      glActiveTexture(GL_TEXTURE1);
      glBindTexture(GL_TEXTURE_2D, weight);
      glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
      glActiveTexture(GL_TEXTURE1);
      glBindTexture(GL_TEXTURE_2D, 0);
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, 0);
    }
    rin = out;
  }
  glDeleteTextures(1, &temp);

#ifdef GLSL4
  free(idx);
#endif

}
