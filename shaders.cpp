#include "shaders.h"

#include <limits>
#include <cstdio>

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
