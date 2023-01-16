// example1
//{{{  includes
#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>

#include <stdio.h>
#include <string.h>

#define FONTSTASH_IMPLEMENTATION
//#define FONS_USE_FREETYPE
#include "fontstash.h"

//#include <GLFW/glfw3.h>
#include "../include/GLFW/glfw3.h"

#define GLFONTSTASH_IMPLEMENTATION
#include "glfontstash.h"
//#include "gl3corefontstash.h"

#pragma comment(lib,"glfw3.lib")
//}}}

namespace {
  float size = 90;
  FONScontext* fonsContext = NULL;
  //{{{
  void expandAtlas (FONScontext* stash) {

    int w = 0, h = 0;

    fonsGetAtlasSize(stash, &w, &h);
    if (w < h)
      w *= 2;
    else
      h *= 2;

    fonsExpandAtlas(stash, w, h);
    printf ("expanded atlas to %d x %d\n", w, h);
    }


  //}}}
  //{{{
  void resetAtlas (FONScontext* stash) {

    fonsResetAtlas (stash, 256,256);
    printf ("reset atlas to 256 x 256\n");
    }
  //}}}
  //{{{
  void stashError (void* uptr, int error, int val) {

    (void)uptr;

    FONScontext* stash = (FONScontext*)uptr;
    switch (error) {
      case FONS_ATLAS_FULL:
        printf ("atlas full\n");
        expandAtlas(stash);
        break;

      case FONS_SCRATCH_FULL:
        printf ("scratch full, tried to allocate %d has %d\n", val, FONS_SCRATCH_BUF_SIZE);
        break;

      case FONS_STATES_OVERFLOW:
        printf ("states overflow\n");
        break;

      case FONS_STATES_UNDERFLOW:
        printf ("statels underflow\n");
        break;
      }
    }
  //}}}
  //{{{
  void dash (float dx, float dy) {

    glBegin(GL_LINES);
    glColor4ub(0,0,0,128);
    glVertex2f(dx-5,dy);
    glVertex2f(dx-10,dy);
    glEnd();
    }
  //}}}
  //{{{
  void line (float sx, float sy, float ex, float ey) {

    glBegin(GL_LINES);
    glColor4ub(0,0,0,128);
    glVertex2f(sx,sy);
    glVertex2f(ex,ey);
    glEnd();
    }
  //}}}
  //{{{
  void key (GLFWwindow* window, int key, int scancode, int action, int mods) {

    (void)scancode;
    (void)mods;

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
      glfwSetWindowShouldClose (window, GL_TRUE);

    if (key == GLFW_KEY_E && action == GLFW_PRESS)
      expandAtlas (fonsContext);

    if (key == GLFW_KEY_R && action == GLFW_PRESS)
      resetAtlas (fonsContext);

    if (key == 265 && action == GLFW_PRESS)
      size += 10;

    if (key == 264 && action == GLFW_PRESS) {
      size -= 10;
      if (size < 20)
        size = 20;
      }
    }
  //}}}
  }

int main() {

  if (!glfwInit())
    return -1;

  const GLFWvidmode* mode = glfwGetVideoMode (glfwGetPrimaryMonitor());
  GLFWwindow* window = glfwCreateWindow (mode->width*3/4, mode->height*3/4, "Font Stash", NULL, NULL);
  if (!window) {
    //{{{  error return
    glfwTerminate();
    return -1;
    }
    //}}}

  glfwSetKeyCallback (window, key);
  glfwMakeContextCurrent (window);

  fonsContext = glfonsCreate (256, 256, FONS_ZERO_TOPLEFT);
  if (fonsContext == NULL) {
    //{{{  error return
    printf ("Could not create stash.\n");
    return -1;
    }
    //}}}

  fonsSetErrorCallback (fonsContext, stashError, fonsContext);

  int fontNormal = fonsAddFont (fonsContext, "sans", "DroidSerif-Regular.ttf");
  if (fontNormal == FONS_INVALID) {
    //{{{  error return
    printf ("Could not add font normal.\n");
    return -1;
    }
    //}}}

  int fontItalic = fonsAddFont (fonsContext, "sans-italic", "DroidSerif-Italic.ttf");
  if (fontItalic == FONS_INVALID) {
    //{{{  error return
    printf("Could not add font italic.\n");
    return -1;
    }
    //}}}

  while (!glfwWindowShouldClose (window)) {
    char msg[64];

    int width;
    int height;
    glfwGetFramebufferSize (window, &width, &height);

    // Update and render
    glViewport (0, 0, width, height);
    glClearColor(0.3f, 0.3f, 0.32f, 1.0f);
    glClear (GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    glEnable (GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable (GL_TEXTURE_2D);
    glMatrixMode (GL_PROJECTION);
    glLoadIdentity();
    glOrtho (0, width, height, 0, -1,1);

    glMatrixMode (GL_MODELVIEW);
    glLoadIdentity();
    glDisable (GL_DEPTH_TEST);
    glColor4ub (255, 255, 255, 255);
    glEnable (GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable (GL_CULL_FACE);

    unsigned int white = glfonsRGBA (255, 255, 255, 255);
    unsigned int brown = glfonsRGBA (192, 128, 0, 128);
    unsigned int blue =  glfonsRGBA (0, 192, 255, 255);
    unsigned int black = glfonsRGBA (0, 0, 0, 255);

    float sx = 50;
    float sy = 50;
    float dx = sx;
    float dy = sy;
    dash (dx,dy);

    fonsClearState (fonsContext);
    fonsSetSize (fonsContext, size);
    fonsSetFont (fonsContext, fontNormal);
    float lh = 0;
    fonsVertMetrics (fonsContext, NULL, NULL, &lh);
    dx = sx;
    dy += lh;
    dash (dx,dy);

    fonsSetSize (fonsContext, size);
    fonsSetFont (fonsContext, fontNormal);
    fonsSetColor (fonsContext, white);
    dx = fonsDrawText (fonsContext, dx,dy,"The quick ",NULL);

    fonsSetSize (fonsContext, size/2);
    fonsSetFont (fonsContext, fontItalic);
    fonsSetColor (fonsContext, brown);
    dx = fonsDrawText (fonsContext, dx,dy,"brown ",NULL);

    fonsSetSize (fonsContext, size/3);
    fonsSetFont (fonsContext, fontNormal);
    fonsSetColor (fonsContext, white);
    dx = fonsDrawText (fonsContext, dx,dy,"fox ",NULL);

    fonsSetSize (fonsContext, 14);
    fonsSetFont (fonsContext, fontNormal);
    fonsSetColor (fonsContext, white);
    fonsDrawText (fonsContext, 20, height-20.f,"Press UP / DOWN keys to change font size and to trigger atlas full callback, R to reset atlas, E to expand atlas.",NULL);

    int atlasw;
    int atlash;
    fonsGetAtlasSize (fonsContext, &atlasw, &atlash);
    snprintf (msg, sizeof(msg), "Atlas: %d × %d", atlasw, atlash);
    fonsDrawText (fonsContext, 20, height-50.f, msg, NULL);

    fonsDrawDebug (fonsContext, width - atlasw - 20.f, 20.0);

    glEnable (GL_DEPTH_TEST);
    glfwSwapBuffers (window);
    glfwPollEvents();
    }

  glfonsDelete (fonsContext);
  glfwTerminate();
  return 0;
  }
