#include "debug.h"
#include <iostream>

using namespace std;

void showCurrentGlError() {
  switch (glGetError()) {
    case GL_NO_ERROR: {
      cout << "GL_NO_ERROR: No error." << endl;
    } break;

    case GL_INVALID_ENUM: {
      cout << "GL_INVALID_ENUM: An unacceptable value is specified for an enumerated argument." << endl;
    } break;

    case GL_INVALID_VALUE: {
      cout << "GL_INVALID_VALUE: A numeric argument is out of range." << endl;
    } break;

    case GL_INVALID_OPERATION: {
      cout << "GL_INVALID_OPERATION: The specified operation is not allowed in the current state." << endl;
    } break;

    case GL_INVALID_FRAMEBUFFER_OPERATION: {
      cout << "GL_INVALID_FRAMEBUFFER_OPERATION: The framebuffer object is not complete." << endl;
    } break;

    case GL_OUT_OF_MEMORY: {
      cout << "GL_OUT_OF_MEMORY: There is not enough memory left to execute the command." << endl;
    } break;

    default: {
      cout << "Unknown error." << endl;
    } break;
  }
}
