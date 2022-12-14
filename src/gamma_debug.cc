#include "gamma_debug.h"

#include <cstdio>

#include "glad/glad.h"

namespace gma::debug {

  void check_gl(const char* file, unsigned int line, const char* expr) {
    GLenum code = glGetError();

    if (code == GL_NO_ERROR) {
      return;
    }

    const char *name = "Unknown";
    const char *desc = "-";

    switch (code) {
      case GL_INVALID_ENUM:
        name = "GL_INVALID_ENUM";
        desc = "An unacceptable value is specified for an enumerated argument.";
        break;
      case GL_INVALID_VALUE:
        name = "GL_INVALID_VALUE";
        desc = "A numeric argument is out of range.";
        break;
      case GL_INVALID_OPERATION:
        name = "GL_INVALID_OPERATION";
        desc = "The specified operation is not allowed in the current state.";
        break;
      case GL_INVALID_FRAMEBUFFER_OPERATION:
        name = "GL_INVALID_FRAMEBUFFER_OPERATION";
        desc = "The command is trying to render to or read from the framebuffer while the currently bound framebuffer is not framebuffer complete.";
        break;
      case GL_OUT_OF_MEMORY:
        name = "GL_OUT_OF_MEMORY";
        desc = "There is not enough memory left to execute the command.";
        break;
      default:
        break;
    }

    std::fprintf(stderr, "Error '%s' at %s:%u for expression '%s': %s\n", name, file, line, expr, desc);
  }

}
