#pragma once

#include <GL/glew.h>

// Compiles and links a GL program using shaders provided as source strings.
// Returns the GL handle for the program if successful, or GL_NONE otherwise.
GLuint create_program(char const* vs, size_t vs_length, char const* fs, size_t fs_length);

// Compiles and links a GL shader progran using shaders loaded from the filesystem.
// Returns the GL handle for the program if successful, or GL_NONE otherwise.
GLuint create_program_from_files(char const* vs_path, char const* fs_path);
