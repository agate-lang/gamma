#ifndef GAMMA_DEBUG_H
#define GAMMA_DEBUG_H

#ifndef NDEBUG
  #define GAMMA_GL_CHECK(expr) do { (expr); debug::check_gl(__FILE__, __LINE__, #expr); } while (false)
  #define GAMMA_GL_CHECK_HERE() debug::check_gl(__FILE__, __LINE__, NULL)
#else
  #define GAMMA_GL_CHECK(expr) (expr)
  #define GAMMA_GL_CHECK_HERE()
#endif

namespace gma::debug {

  void check_gl(const char* file, unsigned int line, const char* expr);

}


#endif // GAMMA_DEBUG_H
