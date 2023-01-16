//{{{
// OpenGL triangle example, simple use of shader
// Copyright (c) Camilla Löwy <elmindreda@glfw.org>
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would
//    be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such, and must not
//    be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source
//    distribution.
//
//========================================================================
//! [code]
//}}}
//{{{  includes
#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>

#include "linmath.h"

#define GLAD_GL_IMPLEMENTATION
#include "glad/gl.h"

#define GLFW_INCLUDE_NONE
#include "../include/GLFW/glfw3.h"

#pragma comment(lib,"glfw3.lib")
//}}}

namespace {
  //{{{
  struct sVertex {
    vec2 pos;
    vec3 col;
    };
  //}}}
  //{{{
  const sVertex vertices[3] = {
    { { -0.6f, -0.4f }, { 1.f, 0.f, 0.f } },
    { {  0.6f, -0.4f }, { 0.f, 1.f, 0.f } },
    { {   0.f,  0.6f }, { 0.f, 0.f, 1.f } }
    };
  //}}}

  const char* vertex_shader_text =
    "#version 330\n"
    "uniform mat4 uMvp;"
    "in vec3 vCol;"
    "in vec2 vPos;"
    "out vec3 color;"
    "void main() {"
      "gl_Position = uMvp * vec4 (vPos, 0.0, 1.0);"
      "color = vCol;"
      "}\n";

  const char* fragment_shader_text =
    "#version 330\n"
    "in vec3 color;"
    "out vec4 fragment;"
    "void main() {"
      "fragment = vec4 (color, 1.0);"
      "}\n";

  //{{{
  void errorCallback (int error, const char* description) {
    fprintf (stderr, "Error: %s\n", description);
    }
  //}}}
  //{{{
  void keyCallback (GLFWwindow* window, int key, int scancode, int action, int mods) {

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
      glfwSetWindowShouldClose (window, GLFW_TRUE);
    }
  //}}}
  }

//{{{
int main() {

  glfwSetErrorCallback (errorCallback);
  if (!glfwInit())
    exit (EXIT_FAILURE);

  glfwWindowHint (GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint (GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint (GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  GLFWwindow* window = glfwCreateWindow (640, 480, "OpenGL Triangle", NULL, NULL);
  if (!window) {
    //{{{  error return
    glfwTerminate();
    exit (EXIT_FAILURE);
    }
    //}}}

  glfwSetKeyCallback (window, keyCallback);

  glfwMakeContextCurrent (window);
  gladLoadGL (glfwGetProcAddress);
  glfwSwapInterval (1);

  // shaders
  const GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource (vertex_shader, 1, &vertex_shader_text, NULL);
  glCompileShader (vertex_shader);

  const GLuint fragment_shader = glCreateShader (GL_FRAGMENT_SHADER);
  glShaderSource (fragment_shader, 1, &fragment_shader_text, NULL);
  glCompileShader (fragment_shader);

  const GLuint program = glCreateProgram();
  glAttachShader (program, vertex_shader);
  glAttachShader (program, fragment_shader);
  glLinkProgram (program);

  glUseProgram (program);

  const GLint mvp_location = glGetUniformLocation (program, "uMvp");
  const GLint vpos_location = glGetAttribLocation (program, "vPos");
  const GLint vcol_location = glGetAttribLocation (program, "vCol");

  // vertices
  GLuint vertex_buffer;
  glGenBuffers (1, &vertex_buffer);
  glBindBuffer (GL_ARRAY_BUFFER, vertex_buffer);
  glBufferData (GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  GLuint vertex_array;
  glGenVertexArrays (1, &vertex_array);
  glBindVertexArray (vertex_array);
  glEnableVertexAttribArray (vpos_location);
  glVertexAttribPointer (vpos_location, 2, GL_FLOAT, GL_FALSE, sizeof(sVertex), (void*)offsetof(sVertex, pos));
  glEnableVertexAttribArray (vcol_location);
  glVertexAttribPointer (vcol_location, 3, GL_FLOAT, GL_FALSE, sizeof(sVertex), (void*)offsetof(sVertex, col));

  // main loop
  while (!glfwWindowShouldClose (window)) {
    int width, height;
    glfwGetFramebufferSize (window, &width, &height);
    const float ratio = width / (float) height;
    glViewport (0, 0, width, height);

    glClear (GL_COLOR_BUFFER_BIT);

    // rotate
    mat4x4 m, p, mvp;
    mat4x4_identity (m);
    mat4x4_rotate_Z (m, m, (float) glfwGetTime());
    mat4x4_ortho (p, -ratio, ratio, -1.f, 1.f, 1.f, -1.f);
    mat4x4_mul (mvp, p, m);
    glUniformMatrix4fv (mvp_location, 1, GL_FALSE, (const GLfloat*)&mvp);

    // draw triangles
    glBindVertexArray (vertex_array);
    glDrawArrays (GL_TRIANGLES, 0, 3);

    glfwSwapBuffers (window);
    glfwPollEvents();
    }

  glfwDestroyWindow (window);

  glfwTerminate();
  exit (EXIT_SUCCESS);
  }
//}}}
