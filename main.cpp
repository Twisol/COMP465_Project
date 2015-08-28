#include "buildconf.h"

// Provides the glm::vec3 class
#include <glm/vec3.hpp>
// GL Extension Wrangler - automatically fetches and assigns OpenGL function pointers
#include <GL/glew.h>
// Cross-platform GL context and window toolkit. Handles the boilerplate.
#include <GLFW/glfw3.h>
// Math library tailored for OpenGL development
#include <glm/glm.hpp>

#include <iostream>

using namespace std;


// A structure representing top-level information about the application.
struct App {
  GLFWwindow* window;  // The GLFW window for this app
  GLuint shader_id;  // The ID of the current shader program.
};

// Store top-level application information as a static singleton, so that
// the GLUT callbacks can access it properly.
static App G_APP;


// Compiles and links a GL program using shaders provided as source strings.
GLuint create_program(char const* vs, size_t vs_length, char const* fs, size_t fs_length) {
  if (vs_length > std::numeric_limits<GLint>::max()) {
    fprintf(stderr, "Vertex shader source too long for OpenGL\n");
    return GL_NONE;
  } else if (fs_length > std::numeric_limits<GLint>::max()) {
    fprintf(stderr, "Fragment shader source too long for OpenGL\n");
    return GL_NONE;
  }

  GLuint program = glCreateProgram();

  {
    GLuint shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(shader, 1, &vs, (GLint const*)&vs_length);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (success != GL_TRUE) {
      GLchar log[1024];
      glGetShaderInfoLog(shader, sizeof(log), NULL, log);

      fprintf(stderr, "Error compiling vertex shader:\n%s\n\n", log);
      return GL_NONE;
    }

    glAttachShader(program, shader);
  }

  {
    GLuint shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(shader, 1, &fs, (GLint const*)&fs_length);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (success != GL_TRUE) {
      GLchar log[1024];
      glGetShaderInfoLog(shader, sizeof(log), NULL, log);

      fprintf(stderr, "Error compiling fragment shader:\n%s\n\n", log);
      return GL_NONE;
    }

    glAttachShader(program, shader);
  }

  glLinkProgram(program);

  {
    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (success != GL_TRUE) {
      GLchar log[1024];
      glGetProgramInfoLog(program, sizeof(log), NULL, log);

      fprintf(stderr, "Error linking shader program:\n%s\n\n", log);
      return GL_NONE;
    }
  }

  return program;
}

// Compiles and links a GL shader progran using shaders loaded from the filesystem.
GLuint create_program_from_files(char const* vs_path, char const* fs_path) {
  char vs[256*1024];  // shader size up to 256 KiB
  size_t vs_length = 0;
  {
    FILE* f = fopen(vs_path, "r");
    vs_length = fread(vs, 1, sizeof(vs), f);
    if (!feof(f)) {
      fprintf(stderr, "Vertex shader too long to load from disk.\n");
      return GL_NONE;
    }
  }

  char fs[256*1024];  // shader size up to 256 KiB
  size_t fs_length = 0;
  {
    FILE* f = fopen(fs_path, "r");
    fs_length = fread(fs, 1, sizeof(fs), f);
    if (!feof(f)) {
      fprintf(stderr, "Fragment shader too long to load from disk.\n");
      return GL_NONE;
    }
  }

  return create_program(vs, vs_length, fs, fs_length);
}


// Renders a frame.
void render() {
  glClear(GL_COLOR_BUFFER_BIT);
  //...

#if GL_SAFETY
  glValidateProgram(G_APP.shader_id);
#endif

  //...

  glfwSwapBuffers(G_APP.window);
}

// Processes ASCII keyboard input.
void keyboard_callback(GLFWwindow* window, int key, int /*scancode*/, int action, int /*mode*/) {
  if (action == GLFW_PRESS && key == GLFW_KEY_ESCAPE) {
    glfwSetWindowShouldClose(window, GL_TRUE);
  }

  //...
}


// A GLFW callback handling GLFW errors
void error_callback(int /*error*/, char const* description) {
  cerr << description << endl;
}

// Entry point.
int main(int /*argc*/, char** /*argv*/) {
  // Initialize GLFW and set an error callback
  if (!glfwInit()) {
    cout << "Unable to initialize GLFW" << endl;
    return 1;
  }

  glfwSetErrorCallback(&error_callback);

  // For the sake of OS X, ensure that we get a modern OpenGL context
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

  // Create a window with the desired dimensions and title
  GLFWwindow* window = glfwCreateWindow(1024, 768, "Project Phase 1", NULL, NULL);
  if (!window) {
    glfwTerminate();
    cout << "Unable to create GLFW window" << endl;
    return 1;
  }
  G_APP.window = window;

  // Mark the OpenGL context as current. This is necessary for any gl* and glew* actions to apply to this window.
  glfwMakeContextCurrent(G_APP.window);

  cout << "Running version " << VERSION << " with OpenGL version " << glGetString(GL_VERSION) << ", GLSL version " << glGetString(GL_SHADING_LANGUAGE_VERSION) << "." << endl;

  // Initialize GLEW, which automatically makes available any OpenGL
  // extensions supported on the system.
  //
  // glewInit must only be called after a GL context is made current.
  glewExperimental = GL_TRUE;
  GLenum glewError = glewInit();
  if (glewError != GLEW_OK) {
    cout << "GLEW could not be initialized." << endl;
    return 1;
  }

  // Initialize our OpenGL rendering context
  {
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    //...

    // Load our shader program
    G_APP.shader_id = create_program_from_files("shaders/vertex.glsl", "shaders/fragment.glsl");
  }

  glfwSetKeyCallback(G_APP.window, &keyboard_callback);

  while (!glfwWindowShouldClose(G_APP.window)) {
    render();
    glfwPollEvents();
  }

  glfwDestroyWindow(G_APP.window);
  G_APP.window = NULL;

  glfwTerminate();
  return 0;
}
