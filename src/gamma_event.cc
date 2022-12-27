#include "gamma_event.h"

#include <cassert>

#include "gamma_agate.h"
#include "gamma_math.h"

namespace gma {

  /*
   * Event
   */

//  NAME
#define GAMMA_EVENT_TYPE_LIST \
  X(QUIT)                     \
  X(RESIZED)                  \
  X(CLOSED)                   \
  X(FOCUS_GAINED)             \
  X(FOCUS_LOST)               \
  X(SHOWN)                    \
  X(HIDDEN)                   \
  X(EXPOSED)                  \
  X(MINIMIZED)                \
  X(MAXIMIZED)                \
  X(RESTORED)                 \
  X(KEY_PRESSED)              \
  X(KEY_REPEATED)             \
  X(KEY_RELEASED)             \
  X(MOUSE_WHEEL_SCROLLED)     \
  X(MOUSE_BUTTON_PRESSED)     \
  X(MOUSE_BUTTON_RELEASED)    \
  X(MOUSE_MOVED)              \
  X(MOUSE_ENTERED)            \
  X(MOUSE_LEFT)               \
  X(GAMEPAD_BUTTON_PRESSED)   \
  X(GAMEPAD_BUTTON_RELEASED)  \
  X(GAMEPAD_AXIS_MOVED)       \
  X(GAMEPAD_CONNECTED)        \
  X(GAMEPAD_DISCONNECTED)     \
  X(TEXT_ENTERED)             \
  X(TOUCH_BEGAN)              \
  X(TOUCH_MOVED)              \
  X(TOUCH_ENDED)              \
  X(UNKNOWN)

  enum class EventType : int64_t {
    #define X(name) name,
    GAMMA_EVENT_TYPE_LIST
    #undef X
  };


  struct EventApi : EventClass {

    static EventType compute_event_kind(const SDL_Event *event) {
      assert(event);

      switch (event->type) {
        case SDL_WINDOWEVENT:
          switch (event->window.event) {
            case SDL_WINDOWEVENT_SIZE_CHANGED:  return EventType::RESIZED;
            case SDL_WINDOWEVENT_CLOSE:         return EventType::CLOSED;
            case SDL_WINDOWEVENT_FOCUS_GAINED:  return EventType::FOCUS_GAINED;
            case SDL_WINDOWEVENT_FOCUS_LOST:    return EventType::FOCUS_LOST;
            case SDL_WINDOWEVENT_ENTER:         return EventType::MOUSE_ENTERED;
            case SDL_WINDOWEVENT_LEAVE:         return EventType::MOUSE_LEFT;
            case SDL_WINDOWEVENT_SHOWN:         return EventType::SHOWN;
            case SDL_WINDOWEVENT_HIDDEN:        return EventType::HIDDEN;
            case SDL_WINDOWEVENT_EXPOSED:       return EventType::EXPOSED;
            case SDL_WINDOWEVENT_MINIMIZED:     return EventType::MINIMIZED;
            case SDL_WINDOWEVENT_MAXIMIZED:     return EventType::MAXIMIZED;
            case SDL_WINDOWEVENT_RESTORED:      return EventType::RESTORED;
            default:
              break;
          }
          break;
        case SDL_QUIT:                    return EventType::QUIT;
        case SDL_KEYDOWN:                 return (event->key.repeat == 0) ? EventType::KEY_PRESSED : EventType::KEY_REPEATED;
        case SDL_KEYUP:                   return EventType::KEY_RELEASED;
        case SDL_MOUSEWHEEL:              return EventType::MOUSE_WHEEL_SCROLLED;
        case SDL_MOUSEBUTTONDOWN:         return EventType::MOUSE_BUTTON_PRESSED;
        case SDL_MOUSEBUTTONUP:           return EventType::MOUSE_BUTTON_RELEASED;
        case SDL_MOUSEMOTION:             return EventType::MOUSE_MOVED;
        case SDL_CONTROLLERDEVICEADDED:   return EventType::GAMEPAD_CONNECTED;
        case SDL_CONTROLLERDEVICEREMOVED: return EventType::GAMEPAD_DISCONNECTED;
        case SDL_CONTROLLERBUTTONDOWN:    return EventType::GAMEPAD_BUTTON_PRESSED;
        case SDL_CONTROLLERBUTTONUP:      return EventType::GAMEPAD_BUTTON_RELEASED;
        case SDL_CONTROLLERAXISMOTION:    return EventType::GAMEPAD_AXIS_MOVED;
        case SDL_TEXTINPUT:               return EventType::TEXT_ENTERED;
        case SDL_FINGERDOWN:              return EventType::TOUCH_BEGAN;
        case SDL_FINGERMOTION:            return EventType::TOUCH_MOVED;
        case SDL_FINGERUP:                return EventType::TOUCH_ENDED;
        default:
          break;
      }

      return EventType::UNKNOWN;
    }

    static void event_new_foreign(AgateVM *vm, const char *class_name, SDL_Event *raw) {
      ptrdiff_t class_slot = agateSlotAllocate(vm);
      agateGetVariable(vm, unit_name, class_name, class_slot);
      Event *event = static_cast<Event *>(agateSlotSetForeign(vm, AGATE_RETURN_SLOT, class_slot));
      *event = *raw;
    }

    static bool event_new(AgateVM *vm, SDL_Event *event) {
      assert(event);

      switch (event->type) {
        case SDL_WINDOWEVENT:
          switch (event->window.event) {
            case SDL_WINDOWEVENT_SIZE_CHANGED:
              event_new_foreign(vm, "ResizeEvent", event);
              return true;
            case SDL_WINDOWEVENT_CLOSE:
            case SDL_WINDOWEVENT_FOCUS_GAINED:
            case SDL_WINDOWEVENT_FOCUS_LOST:
            case SDL_WINDOWEVENT_ENTER:
            case SDL_WINDOWEVENT_LEAVE:
            case SDL_WINDOWEVENT_SHOWN:
            case SDL_WINDOWEVENT_HIDDEN:
            case SDL_WINDOWEVENT_EXPOSED:
            case SDL_WINDOWEVENT_MINIMIZED:
            case SDL_WINDOWEVENT_MAXIMIZED:
            case SDL_WINDOWEVENT_RESTORED:
              event_new_foreign(vm, "__GenericWindowEvent", event);
              return true;
            default:
              return false;
          }
          break;
        case SDL_QUIT:
          event_new_foreign(vm, "__GenericEvent", event);
          return true;
        case SDL_KEYDOWN:
        case SDL_KEYUP:
          event_new_foreign(vm, "KeyboardEvent", event);
          return true;
        case SDL_MOUSEWHEEL:
          event_new_foreign(vm, "MouseWheelEvent", event);
          return true;
        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP:
          event_new_foreign(vm, "MouseButtonEvent", event);
          return true;
        case SDL_MOUSEMOTION:
          event_new_foreign(vm, "MouseMotionEvent", event);
          return true;
        case SDL_CONTROLLERDEVICEADDED:
        case SDL_CONTROLLERDEVICEREMOVED:
        case SDL_CONTROLLERBUTTONDOWN:
        case SDL_CONTROLLERBUTTONUP:
        case SDL_CONTROLLERAXISMOTION:
        case SDL_TEXTINPUT:
        case SDL_FINGERDOWN:
        case SDL_FINGERMOTION:
        case SDL_FINGERUP:
          event_new_foreign(vm, "__GenericEvent", event);
          return true;
        default:
          return false;
      }

      return false;
    }

    // methods

    static void poll(AgateVM *vm) {
      SDL_Event event;

      do {
        if (SDL_PollEvent(&event) == 0) {
          agateSlotSetNil(vm, AGATE_RETURN_SLOT);
          return;
        }
      } while (!event_new(vm, &event));
    }

    static void type(AgateVM *vm) {
      assert(agateCheckTag<EventClass>(vm, 0));
      auto event = agateSlotGet<EventClass>(vm, 0);
      agateSlotSetInt(vm, AGATE_RETURN_SLOT, static_cast<int64_t>(compute_event_kind(event)));
    }

    #define X(name)                 \
    static void name(AgateVM *vm) { \
      agateSlotSetInt(vm, AGATE_RETURN_SLOT, static_cast<int64_t>(EventType::name)); \
    }

    GAMMA_EVENT_TYPE_LIST

    #undef X

  };

  /*
   * KeyboardEvent
   */

  struct KeyboardEventApi : KeyboardEventClass {

    static void window_id(AgateVM *vm) {
      assert(agateCheckTag<KeyboardEventClass>(vm, 0));
      auto event = agateSlotGet<KeyboardEventClass>(vm, 0);
      assert(event->type == SDL_KEYDOWN || event->type == SDL_KEYUP);
      agateSlotSetInt(vm, AGATE_RETURN_SLOT, event->key.windowID);
    }

    static void pressed(AgateVM *vm) {
      assert(agateCheckTag<KeyboardEventClass>(vm, 0));
      auto event = agateSlotGet<KeyboardEventClass>(vm, 0);
      assert(event->type == SDL_KEYDOWN || event->type == SDL_KEYUP);
      agateSlotSetBool(vm, AGATE_RETURN_SLOT, event->key.state == SDL_PRESSED);
    }

    static void repeat(AgateVM *vm) {
      assert(agateCheckTag<KeyboardEventClass>(vm, 0));
      auto event = agateSlotGet<KeyboardEventClass>(vm, 0);
      assert(event->type == SDL_KEYDOWN || event->type == SDL_KEYUP);
      agateSlotSetInt(vm, AGATE_RETURN_SLOT, event->key.repeat);
    }

    static void scancode(AgateVM *vm) {
      assert(agateCheckTag<KeyboardEventClass>(vm, 0));
      auto event = agateSlotGet<KeyboardEventClass>(vm, 0);
      assert(event->type == SDL_KEYDOWN || event->type == SDL_KEYUP);
      agateSlotSetInt(vm, AGATE_RETURN_SLOT, event->key.keysym.scancode);
    }

    static void keycode(AgateVM *vm) {
      assert(agateCheckTag<KeyboardEventClass>(vm, 0));
      auto event = agateSlotGet<KeyboardEventClass>(vm, 0);
      assert(event->type == SDL_KEYDOWN || event->type == SDL_KEYUP);
      agateSlotSetInt(vm, AGATE_RETURN_SLOT, event->key.keysym.sym);
    }

    static void modifiers(AgateVM *vm) {
      assert(agateCheckTag<KeyboardEventClass>(vm, 0));
      auto event = agateSlotGet<KeyboardEventClass>(vm, 0);
      assert(event->type == SDL_KEYDOWN || event->type == SDL_KEYUP);
      agateSlotSetInt(vm, AGATE_RETURN_SLOT, event->key.keysym.mod);
    }

  };

  /*
   * MouseMotionEvent
   */

  struct MouseMotionEventApi : MouseMotionEventClass {

    static void window_id(AgateVM *vm) {
      assert(agateCheckTag<MouseMotionEventClass>(vm, 0));
      auto event = agateSlotGet<MouseMotionEventClass>(vm, 0);
      assert(event->type == SDL_MOUSEMOTION);
      agateSlotSetInt(vm, AGATE_RETURN_SLOT, event->motion.windowID);
    }

    static void coordinates(AgateVM *vm) {
      assert(agateCheckTag<MouseMotionEventClass>(vm, 0));
      auto event = agateSlotGet<MouseMotionEventClass>(vm, 0);
      assert(event->type == SDL_MOUSEMOTION);

      auto coordinates = agateSlotNew<Vec2IClass>(vm, AGATE_RETURN_SLOT);
      coordinates->x = event->motion.x;
      coordinates->y = event->motion.y;
    }

    static void motion(AgateVM *vm) {
      assert(agateCheckTag<MouseMotionEventClass>(vm, 0));
      auto event = agateSlotGet<MouseMotionEventClass>(vm, 0);
      assert(event->type == SDL_MOUSEMOTION);

      auto motion = agateSlotNew<Vec2IClass>(vm, AGATE_RETURN_SLOT);
      motion->x = event->motion.xrel;
      motion->y = event->motion.yrel;
    }

  };

  /*
   * MouseButtonEvent
   */

  struct MouseButtonEventApi : MouseButtonEventClass {

    static void window_id(AgateVM *vm) {
      assert(agateCheckTag<MouseButtonEventClass>(vm, 0));
      auto event = agateSlotGet<MouseButtonEventClass>(vm, 0);
      assert(event->type == SDL_MOUSEBUTTONDOWN || event->type == SDL_MOUSEBUTTONUP);
      agateSlotSetInt(vm, AGATE_RETURN_SLOT, event->button.windowID);
    }

    static void button(AgateVM *vm) {
      assert(agateCheckTag<MouseButtonEventClass>(vm, 0));
      auto event = agateSlotGet<MouseButtonEventClass>(vm, 0);
      assert(event->type == SDL_MOUSEBUTTONDOWN || event->type == SDL_MOUSEBUTTONUP);
      agateSlotSetInt(vm, AGATE_RETURN_SLOT, event->button.button);
    }

    static void pressed(AgateVM *vm) {
      assert(agateCheckTag<MouseButtonEventClass>(vm, 0));
      auto event = agateSlotGet<MouseButtonEventClass>(vm, 0);
      assert(event->type == SDL_MOUSEBUTTONDOWN || event->type == SDL_MOUSEBUTTONUP);
      agateSlotSetBool(vm, AGATE_RETURN_SLOT, event->button.state == SDL_PRESSED);
    }

    static void clicks(AgateVM *vm) {
      assert(agateCheckTag<MouseButtonEventClass>(vm, 0));
      auto event = agateSlotGet<MouseButtonEventClass>(vm, 0);
      assert(event->type == SDL_MOUSEBUTTONDOWN || event->type == SDL_MOUSEBUTTONUP);
      agateSlotSetInt(vm, AGATE_RETURN_SLOT, event->button.clicks);
    }

    static void coordinates(AgateVM *vm) {
      assert(agateCheckTag<MouseButtonEventClass>(vm, 0));
      auto event = agateSlotGet<MouseButtonEventClass>(vm, 0);
      assert(event->type == SDL_MOUSEBUTTONDOWN || event->type == SDL_MOUSEBUTTONUP);

      auto coordinates = agateSlotNew<Vec2IClass>(vm, AGATE_RETURN_SLOT);
      coordinates->x = event->button.x;
      coordinates->y = event->button.y;
    }

  };

  /*
   * MouseWheelEvent
   */

  struct MouseWheelEventApi : MouseWheelEventClass {

    static void window_id(AgateVM *vm) {
      assert(agateCheckTag<MouseWheelEventClass>(vm, 0));
      auto event = agateSlotGet<MouseWheelEventClass>(vm, 0);
      assert(event->type == SDL_MOUSEWHEEL);
      agateSlotSetInt(vm, AGATE_RETURN_SLOT, event->wheel.windowID);
    }

    static void offset(AgateVM *vm) {
      assert(agateCheckTag<MouseWheelEventClass>(vm, 0));
      auto event = agateSlotGet<MouseWheelEventClass>(vm, 0);
      assert(event->type == SDL_MOUSEWHEEL);

      auto offset = agateSlotNew<Vec2IClass>(vm, AGATE_RETURN_SLOT);

      if (event->wheel.direction == SDL_MOUSEWHEEL_NORMAL) {
        offset->x = event->wheel.x;
        offset->y = event->wheel.y;
      } else {
        assert(event->wheel.direction == SDL_MOUSEWHEEL_FLIPPED);
        offset->x = - event->wheel.x;
        offset->y = - event->wheel.y;
      }
    }

  };

  /*
   * WindowEvent
   */

  struct WindowEventApi : WindowEventClass {

    static void window_id(AgateVM *vm) {
      assert(agateCheckTag<WindowEventClass>(vm, 0));
      auto event = agateSlotGet<WindowEventClass>(vm, 0);
      assert(event->type == SDL_WINDOWEVENT);
      agateSlotSetInt(vm, AGATE_RETURN_SLOT, event->window.windowID);
    }

  };

  /*
   * ResizeEvent
   */

  struct ResizeEventApi : ResizeEventClass {

    static void size(AgateVM *vm) {
      assert(agateCheckTag<ResizeEventClass>(vm, 0));
      auto event = agateSlotGet<ResizeEventClass>(vm, 0);
      assert(event->type == SDL_WINDOWEVENT && event->window.event == SDL_WINDOWEVENT_SIZE_CHANGED);

      auto size = agateSlotNew<Vec2IClass>(vm, AGATE_RETURN_SLOT);
      size->x = event->window.data1;
      size->y = event->window.data2;
    }

  };

  /*
   * Mouse
   */

// NAME, CODE
#define GAMMA_MOUSE_BUTTON_LIST \
  X(LEFT,   SDL_BUTTON_LEFT)    \
  X(MIDDLE, SDL_BUTTON_MIDDLE)  \
  X(RIGHT,  SDL_BUTTON_RIGHT)   \
  X(X1,     SDL_BUTTON_X1)      \
  X(X2,     SDL_BUTTON_X2)


  struct MouseApi : MouseClass {
    #define X(name, code)                           \
    static void name(AgateVM *vm) {                 \
      agateSlotSetInt(vm, AGATE_RETURN_SLOT, code); \
    }

    GAMMA_MOUSE_BUTTON_LIST

    #undef X
  };

  /*
   * Scancode
   */

// NAME, CODE
#define GAMMA_SCANCODE_LIST \
  X(UNKNOWN,          SDL_SCANCODE_UNKNOWN)       \
  X(A,                SDL_SCANCODE_A)             \
  X(B,                SDL_SCANCODE_B)             \
  X(C,                SDL_SCANCODE_C)             \
  X(D,                SDL_SCANCODE_D)             \
  X(E,                SDL_SCANCODE_E)             \
  X(F,                SDL_SCANCODE_F)             \
  X(G,                SDL_SCANCODE_G)             \
  X(H,                SDL_SCANCODE_H)             \
  X(I,                SDL_SCANCODE_I)             \
  X(J,                SDL_SCANCODE_J)             \
  X(K,                SDL_SCANCODE_K)             \
  X(L,                SDL_SCANCODE_L)             \
  X(M,                SDL_SCANCODE_M)             \
  X(N,                SDL_SCANCODE_N)             \
  X(O,                SDL_SCANCODE_O)             \
  X(P,                SDL_SCANCODE_P)             \
  X(Q,                SDL_SCANCODE_Q)             \
  X(R,                SDL_SCANCODE_R)             \
  X(S,                SDL_SCANCODE_S)             \
  X(T,                SDL_SCANCODE_T)             \
  X(U,                SDL_SCANCODE_U)             \
  X(V,                SDL_SCANCODE_V)             \
  X(W,                SDL_SCANCODE_W)             \
  X(X,                SDL_SCANCODE_X)             \
  X(Y,                SDL_SCANCODE_Y)             \
  X(Z,                SDL_SCANCODE_Z)             \
  X(NUM1,             SDL_SCANCODE_1)             \
  X(NUM2,             SDL_SCANCODE_2)             \
  X(NUM3,             SDL_SCANCODE_3)             \
  X(NUM4,             SDL_SCANCODE_4)             \
  X(NUM5,             SDL_SCANCODE_5)             \
  X(NUM6,             SDL_SCANCODE_6)             \
  X(NUM7,             SDL_SCANCODE_7)             \
  X(NUM8,             SDL_SCANCODE_8)             \
  X(NUM9,             SDL_SCANCODE_9)             \
  X(NUM0,             SDL_SCANCODE_0)             \
  X(RETURN,           SDL_SCANCODE_RETURN)        \
  X(ESCAPE,           SDL_SCANCODE_ESCAPE)        \
  X(BACKSPACE,        SDL_SCANCODE_BACKSPACE)     \
  X(TAB,              SDL_SCANCODE_TAB)           \
  X(SPACE,            SDL_SCANCODE_SPACE)         \
  X(MINUS,            SDL_SCANCODE_MINUS)         \
  X(EQUALS,           SDL_SCANCODE_EQUALS)        \
  X(LEFT_BRACKET,     SDL_SCANCODE_LEFTBRACKET)   \
  X(RIGHT_BRACKET,    SDL_SCANCODE_RIGHTBRACKET)  \
  X(BACKSLASH,        SDL_SCANCODE_BACKSLASH)     \
  X(SEMICOLON,        SDL_SCANCODE_SEMICOLON)     \
  X(APOSTROPHE,       SDL_SCANCODE_APOSTROPHE)    \
  X(GRAVE,            SDL_SCANCODE_GRAVE)         \
  X(COMMA,            SDL_SCANCODE_COMMA)         \
  X(PERIOD,           SDL_SCANCODE_PERIOD)        \
  X(SLASH,            SDL_SCANCODE_SLASH)         \
  X(CAPS_LOCK,        SDL_SCANCODE_CAPSLOCK)      \
  X(F1,               SDL_SCANCODE_F1)            \
  X(F2,               SDL_SCANCODE_F2)            \
  X(F3,               SDL_SCANCODE_F3)            \
  X(F4,               SDL_SCANCODE_F4)            \
  X(F5,               SDL_SCANCODE_F5)            \
  X(F6,               SDL_SCANCODE_F6)            \
  X(F7,               SDL_SCANCODE_F7)            \
  X(F8,               SDL_SCANCODE_F8)            \
  X(F9,               SDL_SCANCODE_F9)            \
  X(F10,              SDL_SCANCODE_F10)           \
  X(F11,              SDL_SCANCODE_F11)           \
  X(F12,              SDL_SCANCODE_F12)           \
  X(PRINT_SCREEN,     SDL_SCANCODE_PRINTSCREEN)   \
  X(SCROLL_LOCK,      SDL_SCANCODE_SCROLLLOCK)    \
  X(PAUSE,            SDL_SCANCODE_PAUSE)         \
  X(INSERT,           SDL_SCANCODE_INSERT)        \
  X(HOME,             SDL_SCANCODE_HOME)          \
  X(PAGE_UP,          SDL_SCANCODE_PAGEUP)        \
  X(DELETE,           SDL_SCANCODE_DELETE)        \
  X(END,              SDL_SCANCODE_END)           \
  X(PAGE_DOWN,        SDL_SCANCODE_PAGEDOWN)      \
  X(RIGHT,            SDL_SCANCODE_RIGHT)         \
  X(LEFT,             SDL_SCANCODE_LEFT)          \
  X(DOWN,             SDL_SCANCODE_DOWN)          \
  X(UP,               SDL_SCANCODE_UP)            \
  X(NUM_LOCK,         SDL_SCANCODE_NUMLOCKCLEAR)  \
  X(NUMPAD_DIVIDE,    SDL_SCANCODE_KP_DIVIDE)     \
  X(NUMPAD_MULTIPLY,  SDL_SCANCODE_KP_MULTIPLY)   \
  X(NUMPAD_MINUS,     SDL_SCANCODE_KP_MINUS)      \
  X(NUMPAD_PLUS,      SDL_SCANCODE_KP_PLUS)       \
  X(NUMPAD_ENTER,     SDL_SCANCODE_KP_ENTER)      \
  X(NUMPAD_1,         SDL_SCANCODE_KP_1)          \
  X(NUMPAD_2,         SDL_SCANCODE_KP_2)          \
  X(NUMPAD_3,         SDL_SCANCODE_KP_3)          \
  X(NUMPAD_4,         SDL_SCANCODE_KP_4)          \
  X(NUMPAD_5,         SDL_SCANCODE_KP_5)          \
  X(NUMPAD_6,         SDL_SCANCODE_KP_6)          \
  X(NUMPAD_7,         SDL_SCANCODE_KP_7)          \
  X(NUMPAD_8,         SDL_SCANCODE_KP_8)          \
  X(NUMPAD_9,         SDL_SCANCODE_KP_9)          \
  X(NUMPAD_0,         SDL_SCANCODE_KP_0)          \
  X(NUMPAD_PERIOD,    SDL_SCANCODE_KP_PERIOD)     \
  X(APPLICATION,      SDL_SCANCODE_APPLICATION)

  struct ScancodeApi : ScancodeClass {
    #define X(name, code)                           \
    static void name(AgateVM *vm) {                 \
      agateSlotSetInt(vm, AGATE_RETURN_SLOT, code); \
    }

    GAMMA_SCANCODE_LIST

    #undef X
  };

  /*
   * Keycode
   */

// NAME, CODE
#define GAMMA_KEYCODE_LIST \
  X(UNKNOWN,          SDLK_UNKNOWN)       \
  X(RETURN,           SDLK_RETURN)        \
  X(ESCAPE,           SDLK_ESCAPE)        \
  X(BACKSPACE,        SDLK_BACKSPACE)     \
  X(TAB,              SDLK_TAB)           \
  X(SPACE,            SDLK_SPACE)         \
  X(EXCLAIM,          SDLK_EXCLAIM)       \
  X(QUOTE_DOUBLE,     SDLK_QUOTEDBL)      \
  X(HASH,             SDLK_HASH)          \
  X(PERCENT,          SDLK_PERCENT)       \
  X(DOLLAR,           SDLK_DOLLAR)        \
  X(AMPERSAND,        SDLK_AMPERSAND)     \
  X(QUOTE,            SDLK_QUOTE)         \
  X(LEFT_PAREN,       SDLK_LEFTPAREN)     \
  X(RIGHT_PAREN,      SDLK_RIGHTPAREN)    \
  X(ASTERISK,         SDLK_ASTERISK)      \
  X(PLUS,             SDLK_PLUS)          \
  X(COMMA,            SDLK_COMMA)         \
  X(MINUS,            SDLK_MINUS)         \
  X(PERIOD,           SDLK_PERIOD)        \
  X(SLASH,            SDLK_SLASH)         \
  X(NUM0,             SDLK_0)             \
  X(NUM1,             SDLK_1)             \
  X(NUM2,             SDLK_2)             \
  X(NUM3,             SDLK_3)             \
  X(NUM4,             SDLK_4)             \
  X(NUM5,             SDLK_5)             \
  X(NUM6,             SDLK_6)             \
  X(NUM7,             SDLK_7)             \
  X(NUM8,             SDLK_8)             \
  X(NUM9,             SDLK_9)             \
  X(COLON,            SDLK_COLON)         \
  X(SEMICOLON,        SDLK_SEMICOLON)     \
  X(LESS,             SDLK_LESS)          \
  X(EQUALS,           SDLK_EQUALS)        \
  X(GREATER,          SDLK_GREATER)       \
  X(QUESTION,         SDLK_QUESTION)      \
  X(AT,               SDLK_AT)            \
  X(LEFT_BRACKET,     SDLK_LEFTBRACKET)   \
  X(BACKSLASH,        SDLK_BACKSLASH)     \
  X(RIGHT_BRACKET,    SDLK_RIGHTBRACKET)  \
  X(CARET,            SDLK_CARET)         \
  X(UNDERSCORE,       SDLK_UNDERSCORE)    \
  X(BACKQUOTE,        SDLK_BACKQUOTE)     \
  X(A,                SDLK_a)             \
  X(B,                SDLK_b)             \
  X(C,                SDLK_c)             \
  X(D,                SDLK_d)             \
  X(E,                SDLK_e)             \
  X(F,                SDLK_f)             \
  X(G,                SDLK_g)             \
  X(H,                SDLK_h)             \
  X(I,                SDLK_i)             \
  X(J,                SDLK_j)             \
  X(K,                SDLK_k)             \
  X(L,                SDLK_l)             \
  X(M,                SDLK_m)             \
  X(N,                SDLK_n)             \
  X(O,                SDLK_o)             \
  X(P,                SDLK_p)             \
  X(Q,                SDLK_q)             \
  X(R,                SDLK_r)             \
  X(S,                SDLK_s)             \
  X(T,                SDLK_t)             \
  X(U,                SDLK_u)             \
  X(V,                SDLK_v)             \
  X(W,                SDLK_w)             \
  X(X,                SDLK_x)             \
  X(Y,                SDLK_y)             \
  X(Z,                SDLK_z)             \
  X(CAPS_LOCK,        SDLK_CAPSLOCK)      \
  X(F1,               SDLK_F1)            \
  X(F2,               SDLK_F2)            \
  X(F3,               SDLK_F3)            \
  X(F4,               SDLK_F4)            \
  X(F5,               SDLK_F5)            \
  X(F6,               SDLK_F6)            \
  X(F7,               SDLK_F7)            \
  X(F8,               SDLK_F8)            \
  X(F9,               SDLK_F9)            \
  X(F10,              SDLK_F10)           \
  X(F11,              SDLK_F11)           \
  X(F12,              SDLK_F12)           \
  X(PRINT_SCREEN,     SDLK_PRINTSCREEN)   \
  X(SCROLL_LOCK,      SDLK_SCROLLLOCK)    \
  X(PAUSE,            SDLK_PAUSE)         \
  X(INSERT,           SDLK_INSERT)        \
  X(HOME,             SDLK_HOME)          \
  X(PAGE_UP,          SDLK_PAGEUP)        \
  X(DELETE,           SDLK_DELETE)        \
  X(END,              SDLK_END)           \
  X(PAGE_DOWN,        SDLK_PAGEDOWN)      \
  X(RIGHT,            SDLK_RIGHT)         \
  X(LEFT,             SDLK_LEFT)          \
  X(DOWN,             SDLK_DOWN)          \
  X(UP,               SDLK_UP)            \
  X(NUM_LOCK,         SDLK_NUMLOCKCLEAR)  \
  X(NUMPAD_DIVIDE,    SDLK_KP_DIVIDE)     \
  X(NUMPAD_MULTIPLY,  SDLK_KP_MULTIPLY)   \
  X(NUMPAD_MINUS,     SDLK_KP_MINUS)      \
  X(NUMPAD_PLUS,      SDLK_KP_PLUS)       \
  X(NUMPAD_ENTER,     SDLK_KP_ENTER)      \
  X(NUMPAD_1,         SDLK_KP_1)          \
  X(NUMPAD_2,         SDLK_KP_2)          \
  X(NUMPAD_3,         SDLK_KP_3)          \
  X(NUMPAD_4,         SDLK_KP_4)          \
  X(NUMPAD_5,         SDLK_KP_5)          \
  X(NUMPAD_6,         SDLK_KP_6)          \
  X(NUMPAD_7,         SDLK_KP_7)          \
  X(NUMPAD_8,         SDLK_KP_8)          \
  X(NUMPAD_9,         SDLK_KP_9)          \
  X(NUMPAD_0,         SDLK_KP_0)          \
  X(NUMPAD_PERIOD,    SDLK_KP_PERIOD)     \
  X(APPLICATION,      SDLK_APPLICATION)

  struct KeycodeApi : KeycodeClass {
    #define X(name, code)                           \
    static void name(AgateVM *vm) {                 \
      agateSlotSetInt(vm, AGATE_RETURN_SLOT, code); \
    }

    GAMMA_KEYCODE_LIST

    #undef X
  };

  /*
   * Keyboard
   */

  struct KeyboardApi : KeyboardClass {
    static void scancode_name(AgateVM *vm) {
      int64_t scancode;

      if (!agateCheck(vm, 1, scancode)) {
        agateError(vm, "Int parameter expected for `scancode`.");
        return;
      }

      agateSlotSetString(vm, AGATE_RETURN_SLOT, SDL_GetScancodeName(static_cast<SDL_Scancode>(scancode)));
    }

    static void keycode_name(AgateVM *vm) {
      int64_t keycode;

      if (!agateCheck(vm, 1, keycode)) {
        agateError(vm, "Int parameter expected for `keycode`.");
        return;
      }

      agateSlotSetString(vm, AGATE_RETURN_SLOT, SDL_GetKeyName(static_cast<SDL_Keycode>(keycode)));
    }

    static void localize(AgateVM *vm) {
      int64_t scancode;

      if (!agateCheck(vm, 1, scancode)) {
        agateError(vm, "Int parameter expected for `scancode`.");
        return;
      }

      agateSlotSetInt(vm, AGATE_RETURN_SLOT, SDL_GetKeyFromScancode(static_cast<SDL_Scancode>(scancode)));
    }

    static void unlocalize(AgateVM *vm) {
      int64_t keycode;

      if (!agateCheck(vm, 1, keycode)) {
        agateError(vm, "Int parameter expected for `keycode`.");
        return;
      }

      agateSlotSetInt(vm, AGATE_RETURN_SLOT, SDL_GetScancodeFromKey(static_cast<SDL_Keycode>(keycode)));
    }

  };

  /*
   * Modifier
   */

// NAME, CODE
#define GAMMA_MODIFIER_LIST \
  X(NONE,   KMOD_NONE)      \
  X(LSHIFT, KMOD_LSHIFT)    \
  X(RSHIFT, KMOD_RSHIFT)    \
  X(SHIFT,  KMOD_SHIFT)     \
  X(LCTRL,  KMOD_LCTRL)     \
  X(RCTRL,  KMOD_RCTRL)     \
  X(CTRL,   KMOD_CTRL)      \
  X(LALT,   KMOD_LALT)      \
  X(RALT,   KMOD_RALT)      \
  X(ALT,    KMOD_ALT)       \
  X(LGUI,   KMOD_LGUI)      \
  X(RGUI,   KMOD_RGUI)      \
  X(GUI,    KMOD_GUI)       \
  X(NUM,    KMOD_NUM)       \
  X(CAPS,   KMOD_CAPS)      \
  X(MODE,   KMOD_MODE)


  struct ModifierApi : ModifierClass {
    #define X(name, code)                           \
    static void name(AgateVM *vm) {                 \
      agateSlotSetInt(vm, AGATE_RETURN_SLOT, code); \
    }

    GAMMA_MODIFIER_LIST

    #undef X
  };

  void EventUnit::provide_support(Support& support) {
    support.add_class_handler(unit_name, "__GenericEvent", generic_simple_handler<EventClass>());
    support.add_class_handler(unit_name, "KeyboardEvent", generic_simple_handler<KeyboardEventClass>());
    support.add_class_handler(unit_name, "MouseMotionEvent", generic_simple_handler<MouseMotionEventClass>());
    support.add_class_handler(unit_name, "MouseButtonEvent", generic_simple_handler<MouseButtonEventClass>());
    support.add_class_handler(unit_name, "MouseWheelEvent", generic_simple_handler<MouseWheelEventClass>());
    support.add_class_handler(unit_name, "__GenericWindowEvent", generic_simple_handler<WindowEventClass>());
    support.add_class_handler(unit_name, "ResizeEvent", generic_simple_handler<ResizeEventClass>());

    support.add_method(unit_name, EventApi::class_name, AGATE_FOREIGN_METHOD_CLASS, "poll()", EventApi::poll);

    #define X(name) support.add_method(unit_name, EventApi::class_name, AGATE_FOREIGN_METHOD_CLASS, #name, EventApi::name);
    GAMMA_EVENT_TYPE_LIST
    #undef X

    support.add_method(unit_name, EventApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "type", EventApi::type);

    support.add_method(unit_name, KeyboardEventApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "window_id", KeyboardEventApi::window_id);
    support.add_method(unit_name, KeyboardEventApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "pressed", KeyboardEventApi::pressed);
    support.add_method(unit_name, KeyboardEventApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "repeat", KeyboardEventApi::repeat);
    support.add_method(unit_name, KeyboardEventApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "scancode", KeyboardEventApi::scancode);
    support.add_method(unit_name, KeyboardEventApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "keycode", KeyboardEventApi::keycode);
    support.add_method(unit_name, KeyboardEventApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "modifiers", KeyboardEventApi::modifiers);

    support.add_method(unit_name, MouseMotionEventApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "window_id", MouseMotionEventApi::window_id);
    support.add_method(unit_name, MouseMotionEventApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "coordinates", MouseMotionEventApi::coordinates);
    support.add_method(unit_name, MouseMotionEventApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "motion", MouseMotionEventApi::motion);

    support.add_method(unit_name, MouseButtonEventApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "window_id", MouseButtonEventApi::window_id);
    support.add_method(unit_name, MouseButtonEventApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "button", MouseButtonEventApi::button);
    support.add_method(unit_name, MouseButtonEventApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "pressed", MouseButtonEventApi::pressed);
    support.add_method(unit_name, MouseButtonEventApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "clicks", MouseButtonEventApi::clicks);
    support.add_method(unit_name, MouseButtonEventApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "coordinates", MouseButtonEventApi::coordinates);

    support.add_method(unit_name, MouseWheelEventApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "window_id", MouseWheelEventApi::window_id);
    support.add_method(unit_name, MouseWheelEventApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "offset", MouseWheelEventApi::offset);

    support.add_method(unit_name, WindowEventApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "window_id", WindowEventApi::window_id);

    support.add_method(unit_name, ResizeEventApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "size", ResizeEventApi::size);

    #define X(name, code) support.add_method(unit_name, MouseApi::class_name, AGATE_FOREIGN_METHOD_CLASS, #name, MouseApi::name);
    GAMMA_MOUSE_BUTTON_LIST
    #undef X

    #define X(name, code) support.add_method(unit_name, ScancodeApi::class_name, AGATE_FOREIGN_METHOD_CLASS, #name, ScancodeApi::name);
    GAMMA_SCANCODE_LIST
    #undef X

    #define X(name, code) support.add_method(unit_name, KeycodeApi::class_name, AGATE_FOREIGN_METHOD_CLASS, #name, KeycodeApi::name);
    GAMMA_KEYCODE_LIST
    #undef X

    support.add_method(unit_name, KeyboardApi::class_name, AGATE_FOREIGN_METHOD_CLASS, "scancode_name(_)", KeyboardApi::scancode_name);
    support.add_method(unit_name, KeyboardApi::class_name, AGATE_FOREIGN_METHOD_CLASS, "keycode_name(_)", KeyboardApi::keycode_name);
    support.add_method(unit_name, KeyboardApi::class_name, AGATE_FOREIGN_METHOD_CLASS, "localize(_)", KeyboardApi::localize);
    support.add_method(unit_name, KeyboardApi::class_name, AGATE_FOREIGN_METHOD_CLASS, "unlocalize(_)", KeyboardApi::unlocalize);

    #define X(name, code) support.add_method(unit_name, ModifierApi::class_name, AGATE_FOREIGN_METHOD_CLASS, #name, ModifierApi::name);
    GAMMA_MODIFIER_LIST
    #undef X
  }

}
