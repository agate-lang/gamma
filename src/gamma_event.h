#ifndef GAMMA_EVENT_H
#define GAMMA_EVENT_H

#include <SDL2/SDL.h>

#include "gamma_support.h"

namespace gma {

  struct EventUnit {
    static constexpr const char * unit_name = "gamma/event";
    static void provide_support(Support & support);
  };

  using Event = SDL_Event;

  struct EventClass : EventUnit {
    using type = Event;
    static constexpr const char * class_name = "Event";
    static constexpr uint64_t tag = compute_tag(unit_name, class_name);
  };

  struct KeyboardEventClass : EventClass {
    static constexpr const char * class_name = "KeyboardEvent";
  };

  struct MouseMotionEventClass : EventClass {
    static constexpr const char * class_name = "MouseMotionEvent";
  };

  struct MouseButtonEventClass : EventClass {
    static constexpr const char * class_name = "MouseButtonEvent";
  };

  struct MouseWheelEventClass : EventClass {
    static constexpr const char * class_name = "MouseWheelEvent";
  };

  struct GamepadButtonEventClass : EventClass {
    static constexpr const char * class_name = "GamepadButtonEvent";
  };

  struct GamepadAxisEventClass : EventClass {
    static constexpr const char * class_name = "GamepadAxisEvent";
  };

  struct GamepadConnectionEventClass : EventClass {
    static constexpr const char * class_name = "GamepadConnectionEvent";
  };

  struct GamepadDisconnectionEventClass : EventClass {
    static constexpr const char * class_name = "GamepadDisconnectionEvent";
  };

  struct WindowEventClass : EventClass {
    static constexpr const char * class_name = "WindowEvent";
  };

  struct ResizeEventClass : WindowEventClass {
    static constexpr const char * class_name = "ResizeEvent";
  };

  struct MouseClass : EventUnit {
    // no type
    static constexpr const char * class_name = "Mouse";
    static constexpr uint64_t tag = compute_tag(unit_name, class_name);
  };

  struct ScancodeClass : EventUnit {
    // no type
    static constexpr const char * class_name = "Scancode";
    static constexpr uint64_t tag = compute_tag(unit_name, class_name);
  };

  struct KeycodeClass : EventUnit {
    // no type
    static constexpr const char * class_name = "Keycode";
    static constexpr uint64_t tag = compute_tag(unit_name, class_name);
  };

  struct ModifierClass : EventUnit {
    // no type
    static constexpr const char * class_name = "Modifier";
    static constexpr uint64_t tag = compute_tag(unit_name, class_name);
  };

  struct GamepadButtonClass : EventUnit {
    // no type
    static constexpr const char * class_name = "GamepadButton";
    static constexpr uint64_t tag = compute_tag(unit_name, class_name);
  };

  struct GamepadAxisClass : EventUnit {
    // no type
    static constexpr const char * class_name = "GamepadAxis";
    static constexpr uint64_t tag = compute_tag(unit_name, class_name);
  };

}

#endif // GAMMA_EVENT_H
