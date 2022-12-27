#include "gamma_window.h"

#include <cassert>
#include <cstdio>

#include "gamma_agate.h"
#include "gamma_math.h"

namespace gma {

  struct WindowApi : WindowClass {
    static void destroy(AgateVM *vm, const char *unit_name, const char *class_name, void *data) {
      auto window = static_cast<Window *>(data);

      if (window->ptr != nullptr) {
        SDL_DestroyWindow(window->ptr);
        window->ptr = nullptr;
      }
    }

    static void new2(AgateVM *vm) {
      assert(agateCheckTag<WindowClass>(vm, 0));
      auto window = agateSlotGet<WindowClass>(vm, 0);

      const char *title = nullptr;

      if (!agateCheck(vm, 1, title)) {
        agateError(vm, "String parameter expected for `title`.");
        return;
      }

      Vec2I size;

      if (!agateCheck(vm, 2, size)) {
        agateError(vm, "Vec2I parameter expected for `size`.");
        return;
      }

      window->ptr = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, size.x, size.y, SDL_WINDOW_OPENGL);

      if (window->ptr == nullptr) {
        agateError(vm, "Unable to create a window: %s", SDL_GetError());
        return;
      }

      window->should_close = false;
      window->fullscreen = false;
    }

    static void open(AgateVM *vm) {
      assert(agateCheckTag<WindowClass>(vm, 0));
      auto window = agateSlotGet<WindowClass>(vm, 0);
      agateSlotSetBool(vm, AGATE_RETURN_SLOT, !window->should_close);
    }

    static void close(AgateVM *vm) {
      assert(agateCheckTag<WindowClass>(vm, 0));
      auto window = agateSlotGet<WindowClass>(vm, 0);
      window->should_close = true;
      agateSlotSetNil(vm, AGATE_RETURN_SLOT);
    }

    static void id(AgateVM *vm) {
      assert(agateCheckTag<WindowClass>(vm, 0));
      auto window = agateSlotGet<WindowClass>(vm, 0);
      Uint32 id = SDL_GetWindowID(window->ptr);

      if (id == 0) {
        agateError(vm, "Unable to get window id: %s", SDL_GetError());
        agateSlotSetNil(vm, AGATE_RETURN_SLOT);
        return;
      }

      agateSlotSetInt(vm, AGATE_RETURN_SLOT, id);
    }

    static void get_title(AgateVM *vm) {
      assert(agateCheckTag<WindowClass>(vm, 0));
      auto window = agateSlotGet<WindowClass>(vm, 0);
      agateSlotSetString(vm, AGATE_RETURN_SLOT, SDL_GetWindowTitle(window->ptr));
    }

    static void set_title(AgateVM *vm) {
      assert(agateCheckTag<WindowClass>(vm, 0));
      auto window = agateSlotGet<WindowClass>(vm, 0);

      const char *title = nullptr;

      if (!agateCheck(vm, 1, title)) {
        agateError(vm, "String parameter expected for `title`.");
        agateSlotSetNil(vm, AGATE_RETURN_SLOT);
        return;
      }

      SDL_SetWindowTitle(window->ptr, title);
      agateSlotCopy(vm, AGATE_RETURN_SLOT, 1);
    }

    static void get_position(AgateVM *vm) {
      assert(agateCheckTag<WindowClass>(vm, 0));
      auto window = agateSlotGet<WindowClass>(vm, 0);

      auto result = agateSlotNew<Vec2IClass>(vm, AGATE_RETURN_SLOT);
      SDL_GetWindowPosition(window->ptr, &result->x, &result->y);
    }

    static void set_position(AgateVM *vm) {
      assert(agateCheckTag<WindowClass>(vm, 0));
      auto window = agateSlotGet<WindowClass>(vm, 0);

      Vec2I position;

      if (!agateCheck(vm, 1, position)) {
        agateError(vm, "Vec2I parameter expected for `value`.");
        agateSlotSetNil(vm, AGATE_RETURN_SLOT);
        return;
      }

      SDL_SetWindowPosition(window->ptr, position.x, position.y);
      agateSlotCopy(vm, AGATE_RETURN_SLOT, 1);
    }

    static void get_size(AgateVM *vm) {
      assert(agateCheckTag<WindowClass>(vm, 0));
      auto window = agateSlotGet<WindowClass>(vm, 0);

      auto result = agateSlotNew<Vec2IClass>(vm, AGATE_RETURN_SLOT);
      SDL_GetWindowSize(window->ptr, &result->x, &result->y);
    }

    static void set_size(AgateVM *vm) {
      assert(agateCheckTag<WindowClass>(vm, 0));
      auto window = agateSlotGet<WindowClass>(vm, 0);

      Vec2I size;

      if (!agateCheck(vm, 1, size)) {
        agateError(vm, "Vec2I parameter expected for `value`.");
        agateSlotSetNil(vm, AGATE_RETURN_SLOT);
        return;
      }

      SDL_SetWindowSize(window->ptr, size.x, size.y);
      agateSlotCopy(vm, AGATE_RETURN_SLOT, 1);
    }

    static void get_framebuffer_size(AgateVM *vm) {
      assert(agateCheckTag<WindowClass>(vm, 0));
      auto window = agateSlotGet<WindowClass>(vm, 0);

      auto result = agateSlotNew<Vec2IClass>(vm, AGATE_RETURN_SLOT);
      SDL_GL_GetDrawableSize(window->ptr, &result->x, &result->y);
    }

    static void is_fullscreen(AgateVM *vm) {
      assert(agateCheckTag<WindowClass>(vm, 0));
      auto window = agateSlotGet<WindowClass>(vm, 0);
      agateSlotSetBool(vm, AGATE_RETURN_SLOT, window->fullscreen);
    }

    static void update_fullscreen(AgateVM *vm, Window *window, bool fullscreen) {
      if (fullscreen) {
        if (SDL_SetWindowFullscreen(window->ptr, SDL_WINDOW_FULLSCREEN_DESKTOP) != 0) {
          agateError(vm, "Unable to set the window fullscreen.");
        }
      } else {
        if (SDL_SetWindowFullscreen(window->ptr, 0) != 0) {
          agateError(vm, "Unable to come back in windowed mode.");
        }
      }

      window->fullscreen = fullscreen;
    }

    static void set_fullscreen(AgateVM *vm) {
      assert(agateCheckTag<WindowClass>(vm, 0));
      auto window = agateSlotGet<WindowClass>(vm, 0);

      bool fullscreen;

      if (!agateCheck(vm, 1, fullscreen)) {
        agateError(vm, "Bool parameter expected for `value`.");
        agateSlotSetNil(vm, AGATE_RETURN_SLOT);
        return;
      }

      update_fullscreen(vm, window, fullscreen);
      agateSlotCopy(vm, AGATE_RETURN_SLOT, 1);
    }

    static void toggle_fullscreen(AgateVM *vm) {
      assert(agateCheckTag<WindowClass>(vm, 0));
      auto window = agateSlotGet<WindowClass>(vm, 0);

      update_fullscreen(vm, window, !window->fullscreen);
      agateSlotSetNil(vm, AGATE_RETURN_SLOT);
    }

    static void is_minimized(AgateVM *vm) {
      assert(agateCheckTag<WindowClass>(vm, 0));
      auto window = agateSlotGet<WindowClass>(vm, 0);

      Uint32 flags = SDL_GetWindowFlags(window->ptr);
      agateSlotSetBool(vm, AGATE_RETURN_SLOT, (flags & SDL_WINDOW_MINIMIZED) != 0);
    }

    static void minimize(AgateVM *vm) {
      assert(agateCheckTag<WindowClass>(vm, 0));
      auto window = agateSlotGet<WindowClass>(vm, 0);

      SDL_MinimizeWindow(window->ptr);
      agateSlotSetNil(vm, AGATE_RETURN_SLOT);
    }

    static void is_maximized(AgateVM *vm) {
      assert(agateCheckTag<WindowClass>(vm, 0));
      auto window = agateSlotGet<WindowClass>(vm, 0);

      Uint32 flags = SDL_GetWindowFlags(window->ptr);
      agateSlotSetBool(vm, AGATE_RETURN_SLOT, (flags & SDL_WINDOW_MAXIMIZED) != 0);
    }

    static void maximize(AgateVM *vm) {
      assert(agateCheckTag<WindowClass>(vm, 0));
      auto window = agateSlotGet<WindowClass>(vm, 0);

      SDL_MaximizeWindow(window->ptr);
      agateSlotSetNil(vm, AGATE_RETURN_SLOT);
    }

    static void restore(AgateVM *vm) {
      assert(agateCheckTag<WindowClass>(vm, 0));
      auto window = agateSlotGet<WindowClass>(vm, 0);

      SDL_RestoreWindow(window->ptr);
      agateSlotSetNil(vm, AGATE_RETURN_SLOT);
    }

    static void is_visible(AgateVM *vm) {
      assert(agateCheckTag<WindowClass>(vm, 0));
      auto window = agateSlotGet<WindowClass>(vm, 0);

      Uint32 flags = SDL_GetWindowFlags(window->ptr);
      agateSlotSetBool(vm, AGATE_RETURN_SLOT, (flags & SDL_WINDOW_SHOWN) != 0);
    }

    static void set_visible(AgateVM *vm) {
      assert(agateCheckTag<WindowClass>(vm, 0));
      auto window = agateSlotGet<WindowClass>(vm, 0);

      bool visible;

      if (!agateCheck(vm, 1, visible)) {
        agateError(vm, "Bool parameter expected for `value`.");
        agateSlotSetNil(vm, AGATE_RETURN_SLOT);
        return;
      }

      if (visible) {
        SDL_ShowWindow(window->ptr);
      } else {
        SDL_HideWindow(window->ptr);
      }

      agateSlotCopy(vm, AGATE_RETURN_SLOT, 1);
    }

    static void is_decorated(AgateVM *vm) {
      assert(agateCheckTag<WindowClass>(vm, 0));
      auto window = agateSlotGet<WindowClass>(vm, 0);

      Uint32 flags = SDL_GetWindowFlags(window->ptr);
      agateSlotSetBool(vm, AGATE_RETURN_SLOT, (flags & SDL_WINDOW_BORDERLESS) == 0);
    }

    static void set_decorated(AgateVM *vm) {
      assert(agateCheckTag<WindowClass>(vm, 0));
      auto window = agateSlotGet<WindowClass>(vm, 0);

      bool decorated;

      if (!agateCheck(vm, 1, decorated)) {
        agateError(vm, "Bool parameter expected for `value`.");
        agateSlotSetNil(vm, AGATE_RETURN_SLOT);
        return;
      }

      SDL_SetWindowBordered(window->ptr, decorated ? SDL_TRUE : SDL_FALSE);
      agateSlotCopy(vm, AGATE_RETURN_SLOT, 1);
    }

    static void is_resizable(AgateVM *vm) {
      assert(agateCheckTag<WindowClass>(vm, 0));
      auto window = agateSlotGet<WindowClass>(vm, 0);

      Uint32 flags = SDL_GetWindowFlags(window->ptr);
      agateSlotSetBool(vm, AGATE_RETURN_SLOT, (flags & SDL_WINDOW_RESIZABLE) != 0);
    }

    static void set_resizable(AgateVM *vm) {
      assert(agateCheckTag<WindowClass>(vm, 0));
      auto window = agateSlotGet<WindowClass>(vm, 0);

      bool resizable;

      if (!agateCheck(vm, 1, resizable)) {
        agateError(vm, "Bool parameter expected for `value`.");
        agateSlotSetNil(vm, AGATE_RETURN_SLOT);
        return;
      }

      SDL_SetWindowResizable(window->ptr, resizable ? SDL_TRUE : SDL_FALSE);
      agateSlotCopy(vm, AGATE_RETURN_SLOT, 1);
    }

  };


  bool agateCheck(AgateVM *vm, ptrdiff_t slot, Window& result) {
    return false;
  }

  void WindowUnit::provide_support(Support & support) {
    support.add_class_handler(unit_name, WindowClass::class_name, generic_handler<WindowClass>(WindowApi::destroy));

    support.add_method(unit_name, WindowApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "init new(_,_)", WindowApi::new2);
    support.add_method(unit_name, WindowApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "open", WindowApi::open);
    support.add_method(unit_name, WindowApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "close()", WindowApi::close);
    support.add_method(unit_name, WindowApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "id", WindowApi::id);
    support.add_method(unit_name, WindowApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "title", WindowApi::get_title);
    support.add_method(unit_name, WindowApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "title=(_)", WindowApi::set_title);
    support.add_method(unit_name, WindowApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "position", WindowApi::get_position);
    support.add_method(unit_name, WindowApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "position=(_)", WindowApi::set_position);
    support.add_method(unit_name, WindowApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "size", WindowApi::get_size);
    support.add_method(unit_name, WindowApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "size=(_)", WindowApi::set_size);
    support.add_method(unit_name, WindowApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "framebuffer_size", WindowApi::get_framebuffer_size);
    support.add_method(unit_name, WindowApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "fullscreen", WindowApi::is_fullscreen);
    support.add_method(unit_name, WindowApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "fullscreen=(_)", WindowApi::set_fullscreen);
    support.add_method(unit_name, WindowApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "toggle_fullscreen()", WindowApi::toggle_fullscreen);
    support.add_method(unit_name, WindowApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "minimized", WindowApi::is_minimized);
    support.add_method(unit_name, WindowApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "minimize()", WindowApi::minimize);
    support.add_method(unit_name, WindowApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "maximized", WindowApi::is_maximized);
    support.add_method(unit_name, WindowApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "maximize()", WindowApi::maximize);
    support.add_method(unit_name, WindowApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "restore()", WindowApi::restore);
    support.add_method(unit_name, WindowApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "visible", WindowApi::is_visible);
    support.add_method(unit_name, WindowApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "visible=(_)", WindowApi::set_visible);
    support.add_method(unit_name, WindowApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "decorated", WindowApi::is_decorated);
    support.add_method(unit_name, WindowApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "decorated=(_)", WindowApi::set_decorated);
    support.add_method(unit_name, WindowApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "resizable", WindowApi::is_resizable);
    support.add_method(unit_name, WindowApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "resizable=(_)", WindowApi::set_resizable);
  }

}

