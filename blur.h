#ifndef _BLUR_H
#define _BLUR_H

#include <GL4D/gl4dummies.h>

#ifdef __cplusplus
extern "C" {
#endif

  GL4DAPI void GL4DAPIENTRY blurInit(GLuint width, GLuint height);
  GL4DAPI void GL4DAPIENTRY blur(GLuint plate_vao, GLuint radius, GLuint in, GLuint out, GLuint weight, GLuint nb_iterations, int flipV);

#ifdef __cplusplus
}
#endif

#endif
