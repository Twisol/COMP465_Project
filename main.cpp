#include "buildconf.h"

// Provides the glm::vec3 class
#include <glm/vec3.hpp>
// GL Extension Wrangler - automatically fetches and assigns OpenGL function pointers
#include <gl/glew.h>
// Cross-platform GL context and window toolkit. Handles the boilerplate.
#include <gl/glut.h>

#include <iostream>


using namespace std;

void render_scene() {
  glClear(GL_COLOR_BUFFER_BIT);
  glutSwapBuffers();
}

int main(int argc, char** argv) {
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);

  glutInitWindowSize(1024, 768);
  glutInitWindowPosition(100, 100);
  glutCreateWindow("Project Phase 1");

  // Must only be called after a GL context is made current, apparently.
  glewExperimental = true;
  GLenum glewError = glewInit();
  if (glewError != GLEW_OK) {
    cout << "GLEW could not be initialized." << endl;
  }

  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

  glutDisplayFunc(render_scene);
  glutMainLoop();

  cout << "Hello, World!" << endl;
  return 0;
}