#include "buildconf.h"

// Provides the glm::vec3 class
#include <glm/vec3.hpp>
// GL Extension Wrangler - automatically fetches and assigns OpenGL function pointers
#include <gl/glew.h>
// Cross-platform GL context and window toolkit. Handles the boilerplate.
#include <gl/glut.h>
// Math library tailored for OpenGL development
#include <glm/glm.hpp>

#include <iostream>

using namespace std;


static const unsigned char KEY_ESCAPE = '\x1B';


// A structure representing top-level information about the application.
struct App {
  GLint window_id;  // The ID of the window as provided by GLUT.
};

// Store top-level application information as a static singleton, so that
// the GLUT callbacks can access it properly.
static App G_APP;


// Renders a frame.
void render_callback() {
  glClear(GL_COLOR_BUFFER_BIT);

  //...

  glutSwapBuffers();
}

// Processes ASCII keyboard input.
void keyboard_callback(unsigned char key, int /*y*/, int /*x*/) {
  switch (key) {
    case KEY_ESCAPE: {
      glutDestroyWindow(G_APP.window_id);
      exit(0);
    } break;

    //...
  }

  // Posts a "redisplay" message to GLUT's event queue,
  // to be handled on the next iteration of the GLUT main loop.
  glutPostRedisplay();
}

int main(int argc, char** argv) {
  cout << "Running version " << VERSION << "." << endl;

  // Initialize GLUT and tell it the desired properties of our window.
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
  glutInitWindowSize(1024, 768);
  glutInitWindowPosition(100, 100);
  //...

  // We store the window ID in our singleton object
  // so our event callbacks can access it.
  G_APP.window_id = glutCreateWindow("Project Phase 1");

  // Initialize GLEW, which automatically makes available any OpenGL
  // extensions supported on the system.
  //
  // glewInit must only be called after a GL context is made current,
  // in this case by glutCreateWindow.
  glewExperimental = GL_TRUE;
  GLenum glewError = glewInit();
  if (glewError != GLEW_OK) {
    cout << "GLEW could not be initialized." << endl;
  }

  // Initialize our OpenGL rendering context
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  //...

  // Register our event callbacks with the GLUT event loop
  glutDisplayFunc(render_callback);
  glutKeyboardFunc(keyboard_callback);
  //...

  // Kick-start the GLUT event loop.
  // This function never returns.
  glutMainLoop();

  return 0;
}
