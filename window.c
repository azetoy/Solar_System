#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <GL4D/gl4df.h>
#include <GL4D/gl4du.h>
#include <GL4D/gl4duw_SDL2.h>
#include <time.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include "mobile.h"
#include "blur.h"
#include "hide.h"

static void init(void);
static void mouse(int , int , int , int );
static void motion(int , int );
static void draw(void);
static void quit(void);
static void triangle_edge(GLfloat *, int , int , int , int , int );
static void resize(int w, int h);
static void triangle_edge(GLfloat *, int , int , int , int , int );
static void initGL(void);
static void initAudio(const char * );
static void mixCallback(void *, Uint8 *, int );
static void initData(void);


static GLuint _sun = 0,_quad = 0, _bg = 0, _sobel = 0, _pId = 0, _pId2 = 0, _laveId = 0, _plasmaId = 0, _texId = 0;

static GLuint _c0TId = 0, _c1TId = 0, _c2TId = 0, _dTId = 0;
static Mix_Music * _mmusic = NULL;

static int _windowWidth = 800, _windowHeight = 600,_landscape_w = 1025, _landscape_h = 1025;;
static GLuint _shPID = 0,_smPID = 0,_sphere = 0;
static Sint16 _hauteurs[1024];
static GLfloat _plan_s = 5.0f;
static GLuint _fbo = 0;
static GLuint _colorTex = 0;
static GLuint _depthTex = 0;
static GLuint _idTex = 0;
static GLuint _smTex = 0;
static GLuint _nb_mobiles = 20;
static int _picked_mobile = -1;
static GLfloat _picked_mobile_coords[4] = {0};
static GLfloat * _pixels = NULL;
static GLfloat _lumpos[] = { 0, 3, 0, 1 };
static int _wH = 256;
static GLfloat intensite = 1.0f;

#define SHADOW_MAP_SIDE 512
#define EPSILON 0.00000000001

typedef struct cam_t cam_t;
struct cam_t {
    GLfloat x, z;
    GLfloat theta;
};

static cam_t _cam = {0, 0, 0.015};

int main(int argc, char ** argv) {
  if(!gl4duwCreateWindow(argc, argv, "GL4D - Solar System", 0, 0, _windowWidth, _windowHeight, GL4DW_SHOWN))
    return 1;
  if((access("sunrise.jpg",F_OK) || access("lave.png",F_OK)) != 0) {
    extract_file("Sacred_Breath.mp3", "sunrise.jpg", "#ElonMusk");
    extract_file("Sacred_Breath.mp3", "lave.png", "#GiveMeUwU");
  }
  initAudio("Sacred_Breath.mp3");
  init();
  initGL();
  initData();
  atexit(quit);
  gl4duwMouseFunc(mouse);
  gl4duwMotionFunc(motion);
  gl4duwIdleFunc(mobileMove);
  gl4duwDisplayFunc(draw);
  gl4duwMainLoop();
  return 0;
}
static void init(void) {
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

  _shPID  = gl4duCreateProgram("<vs>shaders/basic.vs", "<fs>shaders/basic.fs", NULL);
  _smPID  = gl4duCreateProgram("<vs>shaders/shadowMap.vs", "<fs>shaders/shadowMap.fs", NULL);
  gl4duGenMatrix(GL_FLOAT, "modelMatrix");
  gl4duGenMatrix(GL_FLOAT, "lightViewMatrix");
  gl4duGenMatrix(GL_FLOAT, "lightProjectionMatrix");
  gl4duGenMatrix(GL_FLOAT, "cameraViewMatrix");
  gl4duGenMatrix(GL_FLOAT, "cameraProjectionMatrix");
  gl4duGenMatrix(GL_FLOAT, "cameraPVMMatrix");

  glViewport(0, 0, _windowWidth, _windowHeight);
  gl4duBindMatrix("lightProjectionMatrix");
  gl4duLoadIdentityf();
  gl4duFrustumf(-1, 1, -1, 1, 1.5, 50.0);
  gl4duBindMatrix("cameraProjectionMatrix");
  gl4duLoadIdentityf();
  gl4duFrustumf(-0.5, 0.5, -0.5 * _windowHeight / _windowWidth, 0.5 * _windowHeight / _windowWidth, 1.0, 50.0);
  gl4duBindMatrix("modelMatrix");

  _sphere = gl4dgGenSpheref(30, 30);
  mobileInit(_nb_mobiles, _plan_s, _plan_s);

  glGenTextures(1, &_smTex);
  glBindTexture(GL_TEXTURE_2D, _smTex);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_MAP_SIDE, SHADOW_MAP_SIDE, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

  glGenTextures(1, &_colorTex);
  glBindTexture(GL_TEXTURE_2D, _colorTex);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _windowWidth, _windowHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

  glGenTextures(1, &_depthTex);
  glBindTexture(GL_TEXTURE_2D, _depthTex);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, _windowWidth, _windowHeight, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);

  glGenTextures(1, &_idTex);
  glBindTexture(GL_TEXTURE_2D, _idTex);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, _windowWidth, _windowHeight, 0, GL_RED, GL_UNSIGNED_INT, NULL);

  glGenFramebuffers(1, &_fbo);

  _pixels = malloc(_windowWidth * _windowHeight * sizeof *_pixels);
  assert(_pixels);
}
static void initGL(void) {
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    gl4duGenMatrix(GL_FLOAT, "modelViewMatrix");
    gl4duGenMatrix(GL_FLOAT, "projectionMatrix");
    resize(_windowWidth, _windowHeight);
    blurInit(_windowWidth, _windowHeight);
    glEnable(GL_DEPTH_TEST);
    _pId = gl4duCreateProgram("<vs>shaders/sun.vs", "<fs>shaders/sun.fs", NULL);
    _pId2 = gl4duCreateProgram("<vs>shaders/basicS.vs", "<fs>shaders/mix.fs", NULL);
}
static void initData(void) {
    SDL_Surface * t;
    GLfloat * heightMap = NULL;
    srand(time(NULL));
    _sun = gl4dgGenSpheref(100, 10);
    _quad = gl4dgGenQuadf();
    heightMap = calloc(_landscape_w * _landscape_h, sizeof *heightMap);
    assert(heightMap);
    triangle_edge(heightMap, 0, 0, _landscape_w - 1, _landscape_h - 1, _landscape_w);

    glGenFramebuffers(1, &_fbo);

    glGenTextures(1, &_c0TId);
    glBindTexture(GL_TEXTURE_2D, _c0TId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _windowWidth, _windowHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

    glGenTextures(1, &_c1TId);
    glBindTexture(GL_TEXTURE_2D, _c1TId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _windowWidth, _windowHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

    glGenTextures(1, &_c2TId);
    glBindTexture(GL_TEXTURE_2D, _c2TId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _windowWidth, _windowHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

    glGenTextures(1, &_dTId);
    glBindTexture(GL_TEXTURE_2D, _dTId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, _windowWidth, _windowHeight, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);


    glGenTextures(1, &_plasmaId);
    glBindTexture(GL_TEXTURE_2D, _plasmaId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, _landscape_w, _landscape_h, 0, GL_RED, GL_FLOAT, heightMap);
    free(heightMap);

    glGenTextures(1, &_laveId);
    glBindTexture(GL_TEXTURE_1D, _laveId);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    t = IMG_Load("lave.png");
    assert(t);

    glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB, t->w, 0, t->format->BytesPerPixel == 3 ? GL_RGB : GL_RGBA, GL_UNSIGNED_BYTE, t->pixels);

    SDL_FreeSurface(t);

    glGenTextures(1, &_texId);
    glBindTexture(GL_TEXTURE_2D, _texId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    t = IMG_Load("sunrise.jpg");
    assert(t);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, t->w, t->h, 0, t->format->BytesPerPixel == 3 ? GL_RGB : GL_RGBA, GL_UNSIGNED_BYTE, t->pixels);
    SDL_FreeSurface(t);

    glBindTexture(GL_TEXTURE_1D, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

static void mouse(int button, int state, int x, int y) {
  if(button == GL4D_BUTTON_LEFT) {
    y = _windowHeight - y;
    glBindTexture(GL_TEXTURE_2D, _idTex);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, _pixels);
    if(x >= 0 && x < _windowWidth && y >=0 && y < _windowHeight)
      _picked_mobile = (int)((_nb_mobiles + 2.0f) * _pixels[y * _windowWidth + x]) - 3;
    if(_picked_mobile >= 0 && _picked_mobile < _nb_mobiles) {
      mobileSetFreeze(_picked_mobile, state);
      if(state) {
	GLfloat m[16], tmpp[16], tmpm[16], * gl4dm;
	GLfloat mcoords[] = {0, 0, 0, 1};
	mobileGetCoords(_picked_mobile, mcoords);
	gl4duBindMatrix("cameraProjectionMatrix");
	gl4dm = gl4duGetMatrixData();
	memcpy(tmpp, gl4dm, sizeof tmpp);
	gl4duBindMatrix("cameraViewMatrix");
	gl4dm = gl4duGetMatrixData();
	memcpy(tmpm, gl4dm, sizeof tmpm);
	MMAT4XMAT4(m, tmpp, tmpm);
	MMAT4XVEC4(_picked_mobile_coords, m, mcoords);
	MVEC4WEIGHT(_picked_mobile_coords);
      }
    }
    if(!state)
      _picked_mobile = -1;
  }
}
static void motion(int x, int y) {
  if(_picked_mobile >= 0 && _picked_mobile < _nb_mobiles) {
    GLfloat m[16], tmpp[16], tmpm[16], * gl4dm;
    GLfloat p[] = { 2.0f * x / (GLfloat)_windowWidth - 1.0f,
		    -(2.0f * y / (GLfloat)_windowHeight - 1.0f), 
		    0.0f, 1.0 }, ip[4];
    gl4duBindMatrix("cameraProjectionMatrix");
    gl4dm = gl4duGetMatrixData();
    memcpy(tmpp, gl4dm, sizeof tmpp);
    gl4duBindMatrix("cameraViewMatrix");
    gl4dm = gl4duGetMatrixData();
    memcpy(tmpm, gl4dm, sizeof tmpm);
    MMAT4XMAT4(m, tmpp, tmpm);
    MMAT4INVERSE(m);
    p[2] = _picked_mobile_coords[2];
    MMAT4XVEC4(ip, m, p);
    MVEC4WEIGHT(ip);
    mobileSetCoords(_picked_mobile, ip);
  }
}

static inline void scene(GLboolean sm) {
  glEnable(GL_CULL_FACE);
  if(sm) {
    glCullFace(GL_FRONT);
    glUseProgram(_smPID);
    gl4duBindMatrix("lightViewMatrix");
    gl4duLoadIdentityf();
    gl4duLookAtf(_lumpos[0], _lumpos[1], _lumpos[2], 0, 2, 0, 0, 1, 0);
    gl4duBindMatrix("modelMatrix");
    gl4duLoadIdentityf();
  } else {
    GLfloat vert[] = {0, 1, 0, 1}, lp[4], *mat;
    glCullFace(GL_BACK);
    glUseProgram(_shPID);
    glEnable(GL_TEXTURE_2D);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, _smTex);
    glUniform1i(glGetUniformLocation(_shPID, "smTex"), 0);
    gl4duBindMatrix("cameraViewMatrix");
    gl4duLoadIdentityf();
    gl4duLookAtf(0, 4, 18, 0, 2, 0, 0, 1, 0);
    mat = gl4duGetMatrixData();
    MMAT4XVEC4(lp, mat, _lumpos);
    MVEC4WEIGHT(lp);
    glUniform4fv(glGetUniformLocation(_shPID, "lumpos"), 1, lp);
    gl4duBindMatrix("modelMatrix");
    gl4duLoadIdentityf();
    gl4duPushMatrix(); {
      gl4duTranslatef(_lumpos[0], _lumpos[1], _lumpos[2]);
      gl4duScalef(0.3f, 0.3f, 0.3f);
      gl4duSendMatrices();
    } gl4duPopMatrix();
    glUniform1i(glGetUniformLocation(_shPID, "id"), 2);
    glUniform1i(glGetUniformLocation(_shPID, "nb_mobiles"), _nb_mobiles);
    glUniform4fv(glGetUniformLocation(_shPID, "couleur"), 1, vert);
    glUniform1f(glGetUniformLocation(_shPID, "intensite"), intensite);
    glUniform1i(glGetUniformLocation(_shPID, "id"), 1);
  }
  gl4duPushMatrix(); {
    gl4duRotatef(-90, 1, 0, 0);
    gl4duScalef(_plan_s, _plan_s, _plan_s);
    gl4duSendMatrices();
  } gl4duPopMatrix();
  gl4duSendMatrices();
  mobileDraw(_sphere);
}

static void draw(void) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    intensite += 0.001;
    static GLfloat cycle = 0.0;

    GLenum drawBuffers[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    GLenum renderings[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };

    glBindFramebuffer(GL_FRAMEBUFFER, _fbo);


    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,    GL_TEXTURE_2D, 0, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, _smTex, 0);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, _dTId, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _c0TId, 0 );
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, _c1TId, 0 );
    glDrawBuffers(2, drawBuffers);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    gl4duBindMatrix("modelViewMatrix");
    gl4duLoadIdentityf();
    gl4duLookAtf(_cam.x, 1.0, _cam.z,
                 _cam.x - sin(_cam.theta), 0.95, _cam.z - cos(_cam.theta),
                 0.0, 1.0,0.0);

    glUseProgram(_pId);
    gl4duPushMatrix();
    gl4duTranslatef(0, 0, -50);
    gl4duScalef(5, 5, 5);
    gl4duSendMatrices();
    glUniform1f(glGetUniformLocation(_pId, "cycle"), cycle);
    glUniform1i(glGetUniformLocation(_pId, "plasma"), 0);
    glUniform1i(glGetUniformLocation(_pId, "lave"), 1);
    cycle += 0.001;
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, _plasmaId);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_1D, _laveId);
    gl4dgDraw(_sun);
    glBindTexture(GL_TEXTURE_1D, 0);
    glActiveTexture(GL_TEXTURE0);
    gl4duPopMatrix();

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, 0, 0 );
    glDrawBuffers(1, drawBuffers);

    GLint v[2];
    glGetIntegerv(GL_POLYGON_MODE, v);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    blur(gl4dgGetVAO(_quad), 8, _c0TId, _c0TId, 0, 1, 0);
    blur(gl4dgGetVAO(_quad), 8, _c1TId, _c1TId, 0, 1, 0);

    glUseProgram(0);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebuffer(0, 0, _windowWidth, _windowHeight, 0, 0, _windowWidth, _windowHeight, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, _fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _c2TId, 0 );

    gl4duBindMatrix("modelViewMatrix");
    gl4duLoadIdentityf();
    glUseProgram(_pId2);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, _c0TId);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, _c1TId);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, _texId);
    glUniform1i(glGetUniformLocation(_pId2, "tex0"),  0);
    glUniform1i(glGetUniformLocation(_pId2, "tex1"),  2);
    glUniform1i(glGetUniformLocation(_pId2, "alpha"), 1);
    glUniform1i(glGetUniformLocation(_pId2, "bg"), _bg);
    glUniform1i(glGetUniformLocation(_pId2, "sob"), _sobel);
    glUniform1f(glGetUniformLocation(_pId2, "width"), _windowWidth);
    glUniform1f(glGetUniformLocation(_pId2, "height"), _windowHeight);
    gl4dgDraw(_quad);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE0);

    glUseProgram(0);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebuffer(0, 0, _windowWidth, _windowHeight, 0, 0, _windowWidth, _windowHeight, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    if(v[0] == GL_FILL)
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    else
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    glViewport(0, 0, SHADOW_MAP_SIDE, SHADOW_MAP_SIDE);
    scene(GL_TRUE);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _colorTex, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, _idTex, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, _depthTex, 0);
    glViewport(0, 0, _windowWidth, _windowHeight);
    glDrawBuffers(1, &renderings[1]);
    glDrawBuffers(1, renderings);

    glDrawBuffers(2, renderings);

    scene(GL_FALSE);

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebuffer(0, 0, _windowWidth, _windowHeight, 0, 0, _windowWidth, _windowHeight, GL_COLOR_BUFFER_BIT, GL_LINEAR);
    glBlitFramebuffer(0, 0, _windowWidth, _windowHeight, 0, 0, _windowWidth, _windowHeight, GL_DEPTH_BUFFER_BIT, GL_NEAREST);


}
static void quit(void) {
  if(_fbo) {
    glDeleteTextures(1, &_colorTex);
    glDeleteTextures(1, &_depthTex);
    glDeleteTextures(1, &_idTex);
    glDeleteTextures(1, &_smTex);
    glDeleteFramebuffers(1, &_fbo);
    _fbo = 0;
  }
  if(_pixels) {
    free(_pixels);
    _pixels = NULL;
  }
  gl4duClean(GL4DU_ALL);
}

static void triangle_edge(GLfloat *im, int x, int y, int w, int h, int width) {
    GLint v;
    GLint p[9][2], i, w_2 = w >> 1, w_21 = w_2 + (w&1), h_2 = h >> 1, h_21 = h_2 + (h&1);
    GLfloat ri = w / (GLfloat)width;
    p[0][0] = x;       p[0][1] = y;
    p[1][0] = x + w;   p[1][1] = y;
    p[2][0] = x + w;   p[2][1] = y + h;
    p[3][0] = x;       p[3][1] = y + h;
    p[4][0] = x + w_2; p[4][1] = y;
    p[5][0] = x + w;   p[5][1] = y + h_2;
    p[6][0] = x + w_2; p[6][1] = y + h;
    p[7][0] = x;       p[7][1] = y + h_2;
    p[8][0] = x + w_2; p[8][1] = y + h_2;
    for(i = 4; i < 8; i++) {
        if(im[p[i][0] + p[i][1] * width] >= EPSILON)
            continue;
        im[v = p[i][0] + p[i][1] * width] = (im[p[i - 4][0] + p[i - 4][1] * width] +
                                             im[p[(i - 3) % 4][0] + p[(i - 3) % 4][1] * width]) / 2.0;
        im[v] += gl4dmSURand() * ri;
        im[v] = MIN(MAX(im[v], EPSILON), 1.0);
    }
    if(im[p[i][0] + p[i][1] * width] < EPSILON) {
        im[v = p[8][0] + p[8][1] * width] = (im[p[0][0] + p[0][1] * width] +
                                             im[p[1][0] + p[1][1] * width] +
                                             im[p[2][0] + p[2][1] * width] +
                                             im[p[3][0] + p[3][1] * width]) / 4.0;
        im[v] += gl4dmSURand() * ri * sqrt(2);
        im[v] = MIN(MAX(im[v], 0.0), 1.0);
    }
    if(w_2 > 1 || h_2 > 1)
        triangle_edge(im, p[0][0], p[0][1], w_2, h_2, width);
    if(w_21 > 1 || h_2 > 1)
        triangle_edge(im, p[4][0], p[4][1], w_21, h_2, width);
    if(w_21 > 1 || h_21 > 1)
        triangle_edge(im, p[8][0], p[8][1], w_21, h_21, width);
    if(w_2 > 1 || h_21 > 1)
        triangle_edge(im, p[7][0], p[7][1], w_2, h_21, width);
}

static void initAudio(const char * filename) {
  int mixFlags = MIX_INIT_OGG | MIX_INIT_MP3 | MIX_INIT_MOD;
  Mix_Init(mixFlags);

  if(Mix_OpenAudio(44100, AUDIO_S16LSB, 2, 1024) < 0)
    exit(4);
  if(!(_mmusic = Mix_LoadMUS(filename))) {
    fprintf(stderr, "Erreur lors du Mix_LoadMUS: %s\n", Mix_GetError());
    exit(5);
  }
  Mix_SetPostMix(mixCallback, NULL);
  if(!Mix_PlayingMusic())
    Mix_PlayMusic(_mmusic, 1);
}

static void mixCallback(void *udata, Uint8 *stream, int len) {
  int i;
  Sint16 *s = (Sint16 *)stream;
  if(len >= 2 * 1024)
    for(i = 0; i < 1024; i++)
      _hauteurs[i] = _wH / 2 + (_wH / 2) * s[i] / ((1 << 15) - 1.0);
  return;
}
static void resize(int w, int h) {
    _windowWidth = w;
    _windowHeight = h;
    glViewport(0, 0, _windowWidth, _windowHeight);
    gl4duBindMatrix("projectionMatrix");
    gl4duLoadIdentityf();
    gl4duFrustumf(-0.5, 0.5, -0.5 * _windowHeight / _windowWidth, 0.5 * _windowHeight / _windowWidth, 1.0, 1000.0);
}
