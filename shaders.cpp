#include "shaders.h"

#include <limits>
#include <cstdio>

// Compiles and links a GL program using shaders provided as source strings.
//
// TODO: Leaks shader handles if the shaders or program fail to compile/link.
GLuint create_program(char const* vs, size_t vs_length, char const* fs, size_t fs_length) {
  if (vs_length > std::numeric_limits<GLint>::max()) {
    fprintf(stderr, "Vertex shader source too long for OpenGL\n");
    return GL_NONE;
  } else if (fs_length > std::numeric_limits<GLint>::max()) {
    fprintf(stderr, "Fragment shader source too long for OpenGL\n");
    return GL_NONE;
  }

  GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
  GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
  GLuint program = glCreateProgram();

  {
    glShaderSource(vertex_shader, 1, &vs, (GLint const*)&vs_length);
    glCompileShader(vertex_shader);

    GLint success;
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
    if (success != GL_TRUE) {
      GLchar log[1024];
      glGetShaderInfoLog(vertex_shader, sizeof(log), nullptr, log);

      fprintf(stderr, "Error compiling vertex shader:\n%s\n\n", log);
      return GL_NONE;
    }
  }

  {
    glShaderSource(fragment_shader, 1, &fs, (GLint const*)&fs_length);
    glCompileShader(fragment_shader);

    GLint success;
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
    if (success != GL_TRUE) {
      GLchar log[1024];
      glGetShaderInfoLog(fragment_shader, sizeof(log), nullptr, log);

      fprintf(stderr, "Error compiling fragment shader:\n%s\n\n", log);
      return GL_NONE;
    }
  }

  // Attach the shader to the program, then delete our reference to the shader.
  // The shader remains live, since it's attached to the program.
  glAttachShader(program, vertex_shader);
  glDeleteShader(vertex_shader);

  glAttachShader(program, fragment_shader);
  glDeleteShader(fragment_shader);

  glLinkProgram(program);

  {
    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (success != GL_TRUE) {
      GLchar log[1024];
      glGetProgramInfoLog(program, sizeof(log), nullptr, log);

      fprintf(stderr, "Error linking shader program:\n%s\n\n", log);
      return GL_NONE;
    }
  }

  // Now that linking is complete, release the shader objects.
  glDetachShader(program, vertex_shader);
  glDetachShader(program, fragment_shader);

  return program;
}

// Compiles and links a GL shader program using shaders loaded from the filesystem.
GLuint create_program_from_files(char const* vs_path, char const* fs_path) {
  char vs[256*1024];  // shader size up to 256 KiB
  size_t vs_length = 0;
  {
    FILE* f = fopen(vs_path, "r");
    if (!f) {
      fprintf(stderr, "Unable to open file '%s'.\n", vs_path);
      return GL_NONE;
    }

    vs_length = fread(vs, 1, sizeof(vs), f);
    if (!feof(f)) {
      fprintf(stderr, "Vertex shader too long to load from disk.\n");
      return GL_NONE;
    }

    fclose(f);
  }

  char fs[256*1024];  // shader size up to 256 KiB
  size_t fs_length = 0;
  {
    FILE* f = fopen(fs_path, "r");
    if (!f) {
      fprintf(stderr, "Unable to open file '%s'.\n", fs_path);
      return GL_NONE;
    }

    fs_length = fread(fs, 1, sizeof(fs), f);
    if (!feof(f)) {
      fprintf(stderr, "Fragment shader too long to load from disk.\n");
      return GL_NONE;
    }

    fclose(f);
  }

  return create_program(vs, vs_length, fs, fs_length);
}

bool assertShaderValid(GLuint program) {
#if GL_VALIDATE_SHADERS
  // Ensure that all shader inputs are available, and other such stuff.
  glValidateProgram(program);

  GLint isValid = GL_FALSE;
  glGetProgramiv(program, GL_VALIDATE_STATUS, &isValid);
  if (isValid == GL_FALSE) {
    GLchar log[1024];
    glGetProgramInfoLog(program, sizeof(log), nullptr, log);

    fprintf(stderr, "Error linking shader program:\n%s\n\n", log);
    return false;
  }
#endif

  return true;
}
