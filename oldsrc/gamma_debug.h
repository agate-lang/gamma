#ifndef GAMMA_DEBUG_H
#define GAMMA_DEBUG_H

#ifndef NDEBUG
  #define GAMMA_GL_CHECK(expr) do { (expr); gammaDebugCheckGL(__FILE__, __LINE__, #expr); } while (false)
  #define GAMMA_GL_CHECK_HERE() gammaDebugCheckGL(__FILE__, __LINE__, NULL)
#else
  #define GAMMA_GL_CHECK(expr) (expr)
  #define GAMMA_GL_CHECK_HERE()
#endif

void gammaDebugCheckGL(const char* file, unsigned int line, const char* expr);

#endif // GAMMA_DEBUG_H
