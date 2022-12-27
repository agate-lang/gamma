#ifndef GAMMA_WINDOW_H
#define GAMMA_WINDOW_H

#include <SDL2/SDL.h>

#include "gamma_support.h"

namespace gma {

  struct WindowUnit {
    static constexpr const char * unit_name = "gamma/window";
    static void provide_support(Support & support);
  };

  /*
   * Window
   */

  struct Window {
    SDL_Window *ptr;
    bool should_close;
    bool fullscreen;
  };

  struct WindowClass : WindowUnit {
    using type = Window;
    static constexpr const char * class_name = "Window";
    static constexpr uint64_t tag = compute_tag(unit_name, class_name);
  };

  bool agateCheck(AgateVM *vm, ptrdiff_t slot, Window& result);


}


#endif // GAMMA_WINDOW_H
