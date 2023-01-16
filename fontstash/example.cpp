//{{{  includes
#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>

#include <stdio.h>
#include <string.h>

#define FONTSTASH_IMPLEMENTATION
//#define FONS_USE_FREETYPE
#include "fontstash.h"

#include "../../glfw/include/GLFW/glfw3.h"

#define GLFONTSTASH_IMPLEMENTATION
#include "glfontstash.h"
//#include "gl3corefontstash.h"

#pragma comment(lib,"glfw3.lib")
//}}}

namespace {
  //{{{
  void dash (float dx, float dy) {

    glBegin (GL_LINES);
    glColor4ub (0,0, 0,128);
    glVertex2f (dx-5, dy);
    glVertex2f (dx-10, dy);
    glEnd();
    }
  //}}}
  //{{{
  void line (float sx, float sy, float ex, float ey) {

    glBegin (GL_LINES);
    glColor4ub (0,0, 0,128);
    glVertex2f (sx, sy);
    glVertex2f (ex, ey);
    glEnd();
    }
  //}}}
  //{{{
  void key (GLFWwindow* window, int key, int scancode, int action, int mods) {

    (void)scancode;
    (void)mods;

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
      glfwSetWindowShouldClose (window, GL_TRUE);
    }
  //}}}
  }

int main() {

  if (!glfwInit())
    return -1;

  const GLFWvidmode* mode = glfwGetVideoMode (glfwGetPrimaryMonitor());
  GLFWwindow* window = glfwCreateWindow (mode->width*3/4, mode->height/2, "Font Stash", NULL, NULL);
  if (!window) {
    //{{{  error return
    glfwTerminate();
    return -1;
    }
    //}}}

  glfwSetKeyCallback (window, key);
  glfwMakeContextCurrent (window);

  FONScontext* fonsContext = glfonsCreate (512, 512, FONS_ZERO_TOPLEFT);
  //{{{  error return
  if (fonsContext == NULL) {
    printf("Could not create stash.\n");
    return -1;
    }
  //}}}

  int fontNormal = fonsAddFont (fonsContext, "sans", "DroidSerif-Regular.ttf");
  if (fontNormal == FONS_INVALID) {
    //{{{  error return
    printf("Could not add font normal.\n");
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

  int fontBold = fonsAddFont (fonsContext, "sans-bold", "DroidSerif-Bold.ttf");
  if (fontBold == FONS_INVALID) {
    //{{{  error return
    printf("Could not add font bold.\n");
    return -1;
    }
    //}}}

  int fontJapanese = fonsAddFont (fonsContext, "sans-jp", "DroidSansJapanese.ttf");
  if (fontJapanese == FONS_INVALID) {
    //{{{  error return
    printf("Could not add font japanese.\n");
    return -1;
    }
    //}}}

  while (!glfwWindowShouldClose (window)) {
    int width, height;
    glfwGetFramebufferSize (window, &width, &height);

    // Update and render
    glViewport (0, 0, width, height);
    glClearColor (0.3f, 0.3f, 0.32f, 1.0f);
    glClear (GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    glEnable (GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable (GL_TEXTURE_2D);
    glMatrixMode (GL_PROJECTION);
    glLoadIdentity();
    glOrtho (0,width,height,0,-1,1);

    glMatrixMode (GL_MODELVIEW);
    glLoadIdentity();
    glDisable (GL_DEPTH_TEST);
    glColor4ub (255,255,255,255);
    glEnable (GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    glEnable (GL_CULL_FACE);

    unsigned int white = glfonsRGBA (255, 255, 255, 255);
    unsigned int brown = glfonsRGBA (192, 128, 0, 128);
    unsigned int blue =  glfonsRGBA (0, 192, 255, 255);
    unsigned int black = glfonsRGBA (0, 0, 0, 255);

    float lh = 0;
    float sx = 50;
    float sy = 50;
    float dx = sx;
    float dy = sy;
    dash (dx,dy);

    fonsClearState (fonsContext);
    fonsSetSize (fonsContext, 124.0f);
    fonsSetFont (fonsContext, fontNormal);
    fonsVertMetrics (fonsContext, NULL, NULL, &lh);
    dx = sx;
    dy += lh;
    dash (dx,dy);

    fonsSetSize (fonsContext, 124.0f);
    fonsSetFont (fonsContext, fontNormal);
    fonsSetColor (fonsContext, white);
    dx = fonsDrawText (fonsContext, dx,dy, "The quick ",NULL);

    fonsSetSize (fonsContext, 48.0f);
    fonsSetFont (fonsContext, fontItalic);
    fonsSetColor (fonsContext, brown);
    dx = fonsDrawText (fonsContext, dx,dy, "brown ",NULL);

    fonsSetSize (fonsContext, 24.0f);
    fonsSetFont (fonsContext, fontNormal);
    fonsSetColor (fonsContext, white);
    dx = fonsDrawText (fonsContext, dx,dy, "fox ",NULL);

    fonsVertMetrics (fonsContext, NULL, NULL, &lh);
    dx = sx;
    dy += lh * 1.2f;
    dash (dx,dy);

    fonsSetFont (fonsContext, fontItalic);
    dx = fonsDrawText (fonsContext, dx,dy, "jumps over ",NULL);

    fonsSetFont (fonsContext, fontBold);
    dx = fonsDrawText (fonsContext, dx,dy, "the lazy ",NULL);

    fonsSetFont (fonsContext, fontNormal);
    dx = fonsDrawText (fonsContext, dx,dy, "dog.",NULL);

    dx = sx;
    dy += lh * 1.2f;
    dash (dx,dy);
    fonsSetSize (fonsContext, 12.0f);
    fonsSetFont (fonsContext, fontNormal);
    fonsSetColor (fonsContext, blue);
    fonsDrawText (fonsContext, dx,dy, "Now is the time for all good men to come to the aid of the party.",NULL);

    fonsVertMetrics (fonsContext, NULL,NULL,&lh);
    dx = sx;
    dy += lh * 1.2f * 2;
    dash (dx,dy);
    fonsSetSize (fonsContext, 18.0f);
    fonsSetFont (fonsContext, fontItalic);
    fonsSetColor (fonsContext, white);
    fonsDrawText (fonsContext, dx,dy, "Ég get etið gler án þess að meiða mig.",NULL);

    fonsVertMetrics (fonsContext, NULL,NULL,&lh);
    dx = sx;
    dy += lh * 1.2f;
    dash (dx,dy);
    fonsSetFont (fonsContext, fontJapanese);
    fonsDrawText (fonsContext, dx,dy, "私はガラスを食べられます。それは私を傷つけません。",NULL);

    // Font alignment
    fonsSetSize (fonsContext, 18.0f);
    fonsSetFont (fonsContext, fontNormal);
    fonsSetColor (fonsContext, white);

    dx = 50;
    dy = 350;
    line (dx-10,dy,dx+250,dy);
    fonsSetAlign (fonsContext, FONS_ALIGN_LEFT | FONS_ALIGN_TOP);

    dx = fonsDrawText (fonsContext, dx,dy, "Top",NULL);
    dx += 10;
    fonsSetAlign (fonsContext, FONS_ALIGN_LEFT | FONS_ALIGN_MIDDLE);

    dx = fonsDrawText (fonsContext, dx,dy, "Middle",NULL);
    dx += 10;
    fonsSetAlign (fonsContext, FONS_ALIGN_LEFT | FONS_ALIGN_BASELINE);

    dx = fonsDrawText (fonsContext, dx,dy, "Baseline",NULL);
    dx += 10;
    fonsSetAlign (fonsContext, FONS_ALIGN_LEFT | FONS_ALIGN_BOTTOM);
    fonsDrawText (fonsContext, dx,dy, "Bottom",NULL);

    dx = 150;
    dy = 400;
    line (dx,dy-30,dx,dy+80.0f);
    fonsSetAlign (fonsContext, FONS_ALIGN_LEFT | FONS_ALIGN_BASELINE);
    fonsDrawText (fonsContext, dx,dy, "Left",NULL);

    dy += 30;
    fonsSetAlign (fonsContext, FONS_ALIGN_CENTER | FONS_ALIGN_BASELINE);
    fonsDrawText (fonsContext, dx,dy, "Center",NULL);

    dy += 30;
    fonsSetAlign (fonsContext, FONS_ALIGN_RIGHT | FONS_ALIGN_BASELINE);
    fonsDrawText (fonsContext, dx,dy, "Right",NULL);

    // Blur
    dx = 500;
    dy = 350;
    fonsSetAlign (fonsContext, FONS_ALIGN_LEFT | FONS_ALIGN_BASELINE);

    fonsSetSize (fonsContext, 60.0f);
    fonsSetFont (fonsContext, fontItalic);
    fonsSetColor (fonsContext, white);
    fonsSetSpacing (fonsContext, 5.0f);
    fonsSetBlur (fonsContext, 10.0f);
    fonsDrawText (fonsContext, dx,dy, "Blurry...",NULL);

    dy += 50.0f;
    fonsSetSize (fonsContext, 18.0f);
    fonsSetFont (fonsContext, fontBold);
    fonsSetColor (fonsContext, black);
    fonsSetSpacing (fonsContext, 0.0f);
    fonsSetBlur (fonsContext, 3.0f);
    fonsDrawText (fonsContext, dx,dy+2, "DROP THAT SHADOW",NULL);

    fonsSetColor (fonsContext, white);
    fonsSetBlur (fonsContext, 0);
    fonsDrawText (fonsContext, dx,dy, "DROP THAT SHADOW",NULL);

    fonsDrawDebug (fonsContext, 800.0, 50.0);

    glEnable (GL_DEPTH_TEST);
    glfwSwapBuffers (window);
    glfwPollEvents();
    }

  glfonsDelete (fonsContext);
  glfwTerminate();
  return 0;
  }
