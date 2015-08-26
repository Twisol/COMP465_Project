#include "buildconf.h"

#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#else
#include <gl/gl.h>
#include <gl/glu.h>
#include <gl/glut.h>
#endif

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

  glutDisplayFunc(render_scene);
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

  glutMainLoop();

  cout << "Hello, World!" << endl;
  return 0;
}