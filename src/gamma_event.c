#include "gamma_event.h"

#include <assert.h>

#include <SDL2/SDL.h>

#include "gamma_common.h"
#include "gamma_check.h"
#include "gamma_error.h"
#include "gamma_math.h"
#include "gamma_tags.h"
#include "gamma_utils.h"

/*
 * Event
 */

enum GammaEventType {
  GAMMA_QUIT,

  // window events

  GAMMA_RESIZED,
  GAMMA_CLOSED,
  GAMMA_FOCUS_GAINED,
  GAMMA_FOCUS_LOST,
  GAMMA_SHOWN,
  GAMMA_HIDDEN,
  GAMMA_EXPOSED,
  GAMMA_MINIMIZED,
  GAMMA_MAXIMIZED,
  GAMMA_RESTORED,

  // inputs events

  GAMMA_KEY_PRESSED,
  GAMMA_KEY_REPEATED,
  GAMMA_KEY_RELEASED,

  GAMMA_MOUSE_WHEEL_SCROLLED,
  GAMMA_MOUSE_BUTTON_PRESSED,
  GAMMA_MOUSE_BUTTON_RELEASED,
  GAMMA_MOUSE_MOVED,
  GAMMA_MOUSE_ENTERED,
  GAMMA_MOUSE_LEFT,

  GAMMA_GAMEPAD_BUTTON_PRESSED,
  GAMMA_GAMEPAD_BUTTON_RELEASED,
  GAMMA_GAMEPAD_AXIS_MOVED,
  GAMMA_GAMEPAD_CONNECTED,
  GAMMA_GAMEPAD_DISCONNECTED,

  GAMMA_TEXT_ENTERED,

  GAMMA_TOUCH_BEGAN,
  GAMMA_TOUCH_MOVED,
  GAMMA_TOUCH_ENDED,

  GAMMA_UNKNOWN,
};

struct GammaEvent {
  SDL_Event raw;
};

static enum GammaEventType gammaTranslateEventType(SDL_Event *event) {
  assert(event);

  switch (event->type) {
    case SDL_WINDOWEVENT:
      switch (event->window.event) {
        case SDL_WINDOWEVENT_SIZE_CHANGED:  return GAMMA_RESIZED;
        case SDL_WINDOWEVENT_CLOSE:         return GAMMA_CLOSED;
        case SDL_WINDOWEVENT_FOCUS_GAINED:  return GAMMA_FOCUS_GAINED;
        case SDL_WINDOWEVENT_FOCUS_LOST:    return GAMMA_FOCUS_LOST;
        case SDL_WINDOWEVENT_ENTER:         return GAMMA_MOUSE_ENTERED;
        case SDL_WINDOWEVENT_LEAVE:         return GAMMA_MOUSE_LEFT;
        case SDL_WINDOWEVENT_SHOWN:         return GAMMA_SHOWN;
        case SDL_WINDOWEVENT_HIDDEN:        return GAMMA_HIDDEN;
        case SDL_WINDOWEVENT_EXPOSED:       return GAMMA_EXPOSED;
        case SDL_WINDOWEVENT_MINIMIZED:     return GAMMA_MINIMIZED;
        case SDL_WINDOWEVENT_MAXIMIZED:     return GAMMA_MAXIMIZED;
        case SDL_WINDOWEVENT_RESTORED:      return GAMMA_RESTORED;
        default:
          break;
      }
      break;
    case SDL_QUIT:                    return GAMMA_QUIT;
    case SDL_KEYDOWN:                 return (event->key.repeat == 0) ? GAMMA_KEY_PRESSED : GAMMA_KEY_REPEATED;
    case SDL_KEYUP:                   return GAMMA_KEY_RELEASED;
    case SDL_MOUSEWHEEL:              return GAMMA_MOUSE_WHEEL_SCROLLED;
    case SDL_MOUSEBUTTONDOWN:         return GAMMA_MOUSE_BUTTON_PRESSED;
    case SDL_MOUSEBUTTONUP:           return GAMMA_MOUSE_BUTTON_RELEASED;
    case SDL_MOUSEMOTION:             return GAMMA_MOUSE_MOVED;
    case SDL_CONTROLLERDEVICEADDED:   return GAMMA_GAMEPAD_CONNECTED;
    case SDL_CONTROLLERDEVICEREMOVED: return GAMMA_GAMEPAD_DISCONNECTED;
    case SDL_CONTROLLERBUTTONDOWN:    return GAMMA_GAMEPAD_BUTTON_PRESSED;
    case SDL_CONTROLLERBUTTONUP:      return GAMMA_GAMEPAD_BUTTON_RELEASED;
    case SDL_CONTROLLERAXISMOTION:    return GAMMA_GAMEPAD_AXIS_MOVED;
    case SDL_TEXTINPUT:               return GAMMA_TEXT_ENTERED;
    case SDL_FINGERDOWN:              return GAMMA_TOUCH_BEGAN;
    case SDL_FINGERMOTION:            return GAMMA_TOUCH_MOVED;
    case SDL_FINGERUP:                return GAMMA_TOUCH_ENDED;
    default:
      break;
  }

  return GAMMA_UNKNOWN;
}

static void gammaEventNewForeign(AgateVM *vm, const char *class_name, SDL_Event *raw) {
  ptrdiff_t class_slot = agateSlotAllocate(vm);
  agateGetVariable(vm, "gamma/event", class_name, class_slot);
  struct GammaEvent *event = agateSlotSetForeign(vm, AGATE_RETURN_SLOT, class_slot);
  event->raw = *raw;
}

static bool gammaEventNew(AgateVM *vm, SDL_Event *event) {
  assert(event);

  switch (event->type) {
    case SDL_WINDOWEVENT:
      switch (event->window.event) {
        case SDL_WINDOWEVENT_SIZE_CHANGED:
          gammaEventNewForeign(vm, "ResizeEvent", event);
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
          gammaEventNewForeign(vm, "__GenericWindowEvent", event);
          return true;
        default:
          return false;
      }
      break;
    case SDL_QUIT:
      gammaEventNewForeign(vm, "__GenericEvent", event);
      return true;
    case SDL_KEYDOWN:
    case SDL_KEYUP:
      gammaEventNewForeign(vm, "KeyboardEvent", event);
      return true;
    case SDL_MOUSEWHEEL:
      gammaEventNewForeign(vm, "__GenericEvent", event);
      return true;
    case SDL_MOUSEBUTTONDOWN:
    case SDL_MOUSEBUTTONUP:
      gammaEventNewForeign(vm, "MouseButtonEvent", event);
      return true;
    case SDL_MOUSEMOTION:
      gammaEventNewForeign(vm, "MouseMotionEvent", event);
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
      gammaEventNewForeign(vm, "__GenericEvent", event);
      return true;
    default:
      return false;
  }

  return false;
}

// class

static ptrdiff_t gammaEventAllocate(AgateVM *vm, const char *unit_name, const char *class_name) {
  return sizeof(struct GammaEvent);
}

static uint64_t gammaEventTag(AgateVM *vm, const char *unit_name, const char *class_name) {
  return GAMMA_EVENT_TAG;
}

// methods

static void gammaEventPoll(AgateVM *vm) {
  SDL_Event event;

  do {
    if (SDL_PollEvent(&event) == 0) {
      agateSlotSetNil(vm, AGATE_RETURN_SLOT);
      return;
    }
  } while (!gammaEventNew(vm, &event));
}

static void gammaEventType(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_EVENT_TAG);
  struct GammaEvent *event = agateSlotGetForeign(vm, 0);
  agateSlotSetInt(vm, AGATE_RETURN_SLOT, gammaTranslateEventType(&event->raw));
}

//  NAME,                   FNAME
#define GAMMA_EVENT_TYPE_LIST \
  X(QUIT,                     Quit)                   \
  X(RESIZED,                  Resized)                \
  X(CLOSED,                   Closed)                 \
  X(FOCUS_GAINED,             FocusGained)            \
  X(FOCUS_LOST,               FocusLost)              \
  X(SHOWN,                    Shown)                  \
  X(HIDDEN,                   Hidden)                 \
  X(EXPOSED,                  Exposed)                \
  X(MINIMIZED,                Minimized)              \
  X(MAXIMIZED,                Maximized)              \
  X(RESTORED,                 Restored)               \
  X(KEY_PRESSED,              KeyPressed)             \
  X(KEY_REPEATED,             KeyRepeated)            \
  X(KEY_RELEASED,             KeyReleased)            \
  X(MOUSE_WHEEL_SCROLLED,     MouseWheelScrolled)     \
  X(MOUSE_BUTTON_PRESSED,     MouseButtonPressed)     \
  X(MOUSE_BUTTON_RELEASED,    MouseButtonReleased)    \
  X(MOUSE_MOVED,              MouseMoved)             \
  X(MOUSE_ENTERED,            MouseEntered)           \
  X(MOUSE_LEFT,               MouseLeft)              \
  X(GAMEPAD_BUTTON_PRESSED,   GamepadButtonPressed)   \
  X(GAMEPAD_BUTTON_RELEASED,  GamepadButtonReleased)  \
  X(GAMEPAD_AXIS_MOVED,       GamepadAxisMoved)       \
  X(GAMEPAD_CONNECTED,        GamepadConnected)       \
  X(GAMEPAD_DISCONNECTED,     GamepadDisconnected)    \
  X(TEXT_ENTERED,             TextEntered)            \
  X(TOUCH_BEGAN,              TouchBegan)             \
  X(TOUCH_MOVED,              TouchMoved)             \
  X(TOUCH_ENDED,              TouchEnded)             \
  X(UNKNOWN,                  Unknown)


#define X(name, fname)                                    \
static void gammaEvent ## fname(AgateVM *vm) {            \
  agateSlotSetInt(vm, AGATE_RETURN_SLOT, GAMMA_ ## name); \
}

GAMMA_EVENT_TYPE_LIST

#undef X

/*
 * KeyboardEvent
 */

// methods

static void gammaKeyboardEventWindowId(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_EVENT_TAG);
  struct GammaEvent *event = agateSlotGetForeign(vm, 0);
  assert(event->raw.type == SDL_KEYDOWN || event->raw.type == SDL_KEYUP);
  agateSlotSetInt(vm, AGATE_RETURN_SLOT, event->raw.key.windowID);
}

static void gammaKeyboardEventPressed(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_EVENT_TAG);
  struct GammaEvent *event = agateSlotGetForeign(vm, 0);
  assert(event->raw.type == SDL_KEYDOWN || event->raw.type == SDL_KEYUP);
  agateSlotSetBool(vm, AGATE_RETURN_SLOT, event->raw.key.state == SDL_PRESSED);
}

static void gammaKeyboardEventRepeat(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_EVENT_TAG);
  struct GammaEvent *event = agateSlotGetForeign(vm, 0);
  assert(event->raw.type == SDL_KEYDOWN || event->raw.type == SDL_KEYUP);
  agateSlotSetInt(vm, AGATE_RETURN_SLOT, event->raw.key.repeat);
}

static void gammaKeyboardEventScancode(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_EVENT_TAG);
  struct GammaEvent *event = agateSlotGetForeign(vm, 0);
  assert(event->raw.type == SDL_KEYDOWN || event->raw.type == SDL_KEYUP);
  agateSlotSetInt(vm, AGATE_RETURN_SLOT, event->raw.key.keysym.scancode);
}

static void gammaKeyboardEventKeycode(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_EVENT_TAG);
  struct GammaEvent *event = agateSlotGetForeign(vm, 0);
  assert(event->raw.type == SDL_KEYDOWN || event->raw.type == SDL_KEYUP);
  agateSlotSetInt(vm, AGATE_RETURN_SLOT, event->raw.key.keysym.sym);
}

static void gammaKeyboardEventModifiers(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_EVENT_TAG);
  struct GammaEvent *event = agateSlotGetForeign(vm, 0);
  assert(event->raw.type == SDL_KEYDOWN || event->raw.type == SDL_KEYUP);
  agateSlotSetInt(vm, AGATE_RETURN_SLOT, event->raw.key.keysym.mod);
}

/*
 * MouseMotionEvent
 */

static void gammaMouseMotionEventWindowId(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_EVENT_TAG);
  struct GammaEvent *event = agateSlotGetForeign(vm, 0);
  assert(event->raw.type == SDL_MOUSEMOTION);
  agateSlotSetInt(vm, AGATE_RETURN_SLOT, event->raw.motion.windowID);
}

static void gammaMouseMotionEventCoordinates(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_EVENT_TAG);
  struct GammaEvent *event = agateSlotGetForeign(vm, 0);
  assert(event->raw.type == SDL_MOUSEMOTION);

  ptrdiff_t class_slot = agateSlotAllocate(vm);
  agateGetVariable(vm, "gamma", "Vec2I", class_slot);
  struct GammaVec2I *coords = agateSlotSetForeign(vm, AGATE_RETURN_SLOT, class_slot);

  coords->v[0] = event->raw.motion.x;
  coords->v[1] = event->raw.motion.y;
}

static void gammaMouseMotionEventMotion(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_EVENT_TAG);
  struct GammaEvent *event = agateSlotGetForeign(vm, 0);
  assert(event->raw.type == SDL_MOUSEMOTION);

  ptrdiff_t class_slot = agateSlotAllocate(vm);
  agateGetVariable(vm, "gamma", "Vec2I", class_slot);
  struct GammaVec2I *motion = agateSlotSetForeign(vm, AGATE_RETURN_SLOT, class_slot);

  motion->v[0] = event->raw.motion.xrel;
  motion->v[1] = event->raw.motion.yrel;
}

/*
 * MouseMotionEvent
 */

static void gammaMouseButtonEventWindowId(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_EVENT_TAG);
  struct GammaEvent *event = agateSlotGetForeign(vm, 0);
  assert(event->raw.type == SDL_MOUSEBUTTONDOWN || event->raw.type == SDL_MOUSEBUTTONUP);
  agateSlotSetInt(vm, AGATE_RETURN_SLOT, event->raw.button.windowID);
}

static void gammaMouseButtonEventButton(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_EVENT_TAG);
  struct GammaEvent *event = agateSlotGetForeign(vm, 0);
  assert(event->raw.type == SDL_MOUSEBUTTONDOWN || event->raw.type == SDL_MOUSEBUTTONUP);
  agateSlotSetInt(vm, AGATE_RETURN_SLOT, event->raw.button.button);
}

static void gammaMouseButtonEventPressed(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_EVENT_TAG);
  struct GammaEvent *event = agateSlotGetForeign(vm, 0);
  assert(event->raw.type == SDL_MOUSEBUTTONDOWN || event->raw.type == SDL_MOUSEBUTTONUP);
  agateSlotSetBool(vm, AGATE_RETURN_SLOT, event->raw.button.state == SDL_PRESSED);
}

static void gammaMouseButtonEventClicks(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_EVENT_TAG);
  struct GammaEvent *event = agateSlotGetForeign(vm, 0);
  assert(event->raw.type == SDL_MOUSEBUTTONDOWN || event->raw.type == SDL_MOUSEBUTTONUP);
  agateSlotSetInt(vm, AGATE_RETURN_SLOT, event->raw.button.clicks);
}

static void gammaMouseButtonEventCoordinates(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_EVENT_TAG);
  struct GammaEvent *event = agateSlotGetForeign(vm, 0);
  assert(event->raw.type == SDL_MOUSEBUTTONDOWN || event->raw.type == SDL_MOUSEBUTTONUP);

  struct GammaVec2I *coords = gammaForeignAllocate(vm, AGATE_RETURN_SLOT, "gamma/math", "Vec2I");
  coords->v[0] = event->raw.button.x;
  coords->v[1] = event->raw.button.y;
}

/*
 * MouseWheelEvent
 */

static void gammaMouseWheelEventWindowId(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_EVENT_TAG);
  struct GammaEvent *event = agateSlotGetForeign(vm, 0);
  assert(event->raw.type == SDL_MOUSEWHEEL);
  agateSlotSetInt(vm, AGATE_RETURN_SLOT, event->raw.wheel.windowID);
}

static void gammaMouseWheelEventOffset(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_EVENT_TAG);
  struct GammaEvent *event = agateSlotGetForeign(vm, 0);
  assert(event->raw.type == SDL_MOUSEWHEEL);

  struct GammaVec2I *offset = gammaForeignAllocate(vm, AGATE_RETURN_SLOT, "gamma/math", "Vec2I");

  if (event->raw.wheel.direction == SDL_MOUSEWHEEL_NORMAL) {
    offset->v[0] = event->raw.wheel.x;
    offset->v[1] = event->raw.wheel.y;
  } else {
    assert(event->raw.wheel.direction == SDL_MOUSEWHEEL_FLIPPED);
    offset->v[0] = - event->raw.wheel.x;
    offset->v[1] = - event->raw.wheel.y;
  }
}

/*
 * WindowEvent
 */

static void gammaWindowEventWindowId(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_EVENT_TAG);
  struct GammaEvent *event = agateSlotGetForeign(vm, 0);
  assert(event->raw.type == SDL_WINDOWEVENT);
  agateSlotSetInt(vm, AGATE_RETURN_SLOT, event->raw.key.windowID);
}

/*
 * ResizeEvent
 */

static void gammaResizeEventSize(AgateVM *vm) {
  assert(agateSlotGetForeignTag(vm, 0) == GAMMA_EVENT_TAG);
  struct GammaEvent *event = agateSlotGetForeign(vm, 0);
  assert(event->raw.type == SDL_WINDOWEVENT && event->raw.window.event == SDL_WINDOWEVENT_SIZE_CHANGED);

  struct GammaVec2I *size = gammaForeignAllocate(vm, AGATE_RETURN_SLOT, "gamma/math", "Vec2I");
  size->v[0] = event->raw.window.data1;
  size->v[1] = event->raw.window.data2;
}

/*
 * Mouse
 */

// NAME, FNAME, CODE
#define GAMMA_MOUSE_BUTTON_LIST         \
  X(LEFT,   Left,   SDL_BUTTON_LEFT)    \
  X(MIDDLE, Middle, SDL_BUTTON_MIDDLE)  \
  X(RIGHT,  Right,  SDL_BUTTON_RIGHT)   \
  X(X1,     X1,     SDL_BUTTON_X1)      \
  X(X2,     X2,     SDL_BUTTON_X2)


#define X(name, fname, code)                    \
static void gammaMouse ## fname(AgateVM *vm) {  \
  agateSlotSetInt(vm, AGATE_RETURN_SLOT, code); \
}

GAMMA_MOUSE_BUTTON_LIST

#undef X

/*
 * Scancode
 */

// NAME, FNAME, CODE
#define GAMMA_SCANCODE_LIST \
  X(UNKNOWN,          Unknown,        SDL_SCANCODE_UNKNOWN)       \
  X(A,                A,              SDL_SCANCODE_A)             \
  X(B,                B,              SDL_SCANCODE_B)             \
  X(C,                C,              SDL_SCANCODE_C)             \
  X(D,                D,              SDL_SCANCODE_D)             \
  X(E,                E,              SDL_SCANCODE_E)             \
  X(F,                F,              SDL_SCANCODE_F)             \
  X(G,                G,              SDL_SCANCODE_G)             \
  X(H,                H,              SDL_SCANCODE_H)             \
  X(I,                I,              SDL_SCANCODE_I)             \
  X(J,                J,              SDL_SCANCODE_J)             \
  X(K,                K,              SDL_SCANCODE_K)             \
  X(L,                L,              SDL_SCANCODE_L)             \
  X(M,                M,              SDL_SCANCODE_M)             \
  X(N,                N,              SDL_SCANCODE_N)             \
  X(O,                O,              SDL_SCANCODE_O)             \
  X(P,                P,              SDL_SCANCODE_P)             \
  X(Q,                Q,              SDL_SCANCODE_Q)             \
  X(R,                R,              SDL_SCANCODE_R)             \
  X(S,                S,              SDL_SCANCODE_S)             \
  X(T,                T,              SDL_SCANCODE_T)             \
  X(U,                U,              SDL_SCANCODE_U)             \
  X(V,                V,              SDL_SCANCODE_V)             \
  X(W,                W,              SDL_SCANCODE_W)             \
  X(X,                X,              SDL_SCANCODE_X)             \
  X(Y,                Y,              SDL_SCANCODE_Y)             \
  X(Z,                Z,              SDL_SCANCODE_Z)             \
  X(NUM1,             Num1,           SDL_SCANCODE_1)             \
  X(NUM2,             Num2,           SDL_SCANCODE_2)             \
  X(NUM3,             Num3,           SDL_SCANCODE_3)             \
  X(NUM4,             Num4,           SDL_SCANCODE_4)             \
  X(NUM5,             Num5,           SDL_SCANCODE_5)             \
  X(NUM6,             Num6,           SDL_SCANCODE_6)             \
  X(NUM7,             Num7,           SDL_SCANCODE_7)             \
  X(NUM8,             Num8,           SDL_SCANCODE_8)             \
  X(NUM9,             Num9,           SDL_SCANCODE_9)             \
  X(NUM0,             Num0,           SDL_SCANCODE_0)             \
  X(RETURN,           Return,         SDL_SCANCODE_RETURN)        \
  X(ESCAPE,           Escape,         SDL_SCANCODE_ESCAPE)        \
  X(BACKSPACE,        Backspace,      SDL_SCANCODE_BACKSPACE)     \
  X(TAB,              Tab,            SDL_SCANCODE_TAB)           \
  X(SPACE,            Space,          SDL_SCANCODE_SPACE)         \
  X(MINUS,            Minus,          SDL_SCANCODE_MINUS)         \
  X(EQUALS,           Equals,         SDL_SCANCODE_EQUALS)        \
  X(LEFT_BRACKET,     LeftBracket,    SDL_SCANCODE_LEFTBRACKET)   \
  X(RIGHT_BRACKET,    RightBracket,   SDL_SCANCODE_RIGHTBRACKET)  \
  X(BACKSLASH,        Backslash,      SDL_SCANCODE_BACKSLASH)     \
  X(SEMICOLON,        Semicolon,      SDL_SCANCODE_SEMICOLON)     \
  X(APOSTROPHE,       Apostrophe,     SDL_SCANCODE_APOSTROPHE)    \
  X(GRAVE,            Grave,          SDL_SCANCODE_GRAVE)         \
  X(COMMA,            Comma,          SDL_SCANCODE_COMMA)         \
  X(PERIOD,           Period,         SDL_SCANCODE_PERIOD)        \
  X(SLASH,            Slash,          SDL_SCANCODE_SLASH)         \
  X(CAPS_LOCK,        CapsLock,       SDL_SCANCODE_CAPSLOCK)      \
  X(F1,               F1,             SDL_SCANCODE_F1)            \
  X(F2,               F2,             SDL_SCANCODE_F2)            \
  X(F3,               F3,             SDL_SCANCODE_F3)            \
  X(F4,               F4,             SDL_SCANCODE_F4)            \
  X(F5,               F5,             SDL_SCANCODE_F5)            \
  X(F6,               F6,             SDL_SCANCODE_F6)            \
  X(F7,               F7,             SDL_SCANCODE_F7)            \
  X(F8,               F8,             SDL_SCANCODE_F8)            \
  X(F9,               F9,             SDL_SCANCODE_F9)            \
  X(F10,              F10,            SDL_SCANCODE_F10)           \
  X(F11,              F11,            SDL_SCANCODE_F11)           \
  X(F12,              F12,            SDL_SCANCODE_F12)           \
  X(PRINT_SCREEN,     PrintScreen,    SDL_SCANCODE_PRINTSCREEN)   \
  X(SCROLL_LOCK,      ScrollLock,     SDL_SCANCODE_SCROLLLOCK)    \
  X(PAUSE,            Pause,          SDL_SCANCODE_PAUSE)         \
  X(INSERT,           Insert,         SDL_SCANCODE_INSERT)        \
  X(HOME,             Home,           SDL_SCANCODE_HOME)          \
  X(PAGE_UP,          PageUp,         SDL_SCANCODE_PAGEUP)        \
  X(DELETE,           Delete,         SDL_SCANCODE_DELETE)        \
  X(END,              End,            SDL_SCANCODE_END)           \
  X(PAGE_DOWN,        PageDown,       SDL_SCANCODE_PAGEDOWN)      \
  X(RIGHT,            Right,          SDL_SCANCODE_RIGHT)         \
  X(LEFT,             Left,           SDL_SCANCODE_LEFT)          \
  X(DOWN,             Down,           SDL_SCANCODE_DOWN)          \
  X(UP,               Up,             SDL_SCANCODE_UP)            \
  X(NUM_LOCK,         NumLock,        SDL_SCANCODE_NUMLOCKCLEAR)  \
  X(NUMPAD_DIVIDE,    NumpadDivide,   SDL_SCANCODE_KP_DIVIDE)     \
  X(NUMPAD_MULTIPLY,  NumpadMultiply, SDL_SCANCODE_KP_MULTIPLY)   \
  X(NUMPAD_MINUS,     NumpadMinus,    SDL_SCANCODE_KP_MINUS)      \
  X(NUMPAD_PLUS,      NumpadPlus,     SDL_SCANCODE_KP_PLUS)       \
  X(NUMPAD_ENTER,     NumpadEnter,    SDL_SCANCODE_KP_ENTER)      \
  X(NUMPAD_1,         Numpad1,        SDL_SCANCODE_KP_1)          \
  X(NUMPAD_2,         Numpad2,        SDL_SCANCODE_KP_2)          \
  X(NUMPAD_3,         Numpad3,        SDL_SCANCODE_KP_3)          \
  X(NUMPAD_4,         Numpad4,        SDL_SCANCODE_KP_4)          \
  X(NUMPAD_5,         Numpad5,        SDL_SCANCODE_KP_5)          \
  X(NUMPAD_6,         Numpad6,        SDL_SCANCODE_KP_6)          \
  X(NUMPAD_7,         Numpad7,        SDL_SCANCODE_KP_7)          \
  X(NUMPAD_8,         Numpad8,        SDL_SCANCODE_KP_8)          \
  X(NUMPAD_9,         Numpad9,        SDL_SCANCODE_KP_9)          \
  X(NUMPAD_0,         Numpad0,        SDL_SCANCODE_KP_0)          \
  X(NUMPAD_PERIOD,    NumpadPeriod,   SDL_SCANCODE_KP_PERIOD)     \
  X(APPLICATION,      Application,    SDL_SCANCODE_APPLICATION)


#define X(name, fname, code)                      \
static void gammaScancode ## fname(AgateVM *vm) { \
  agateSlotSetInt(vm, AGATE_RETURN_SLOT, code);   \
}

GAMMA_SCANCODE_LIST

#undef X

/*
 * Keycode
 */

// NAME, FNAME, CODE
#define GAMMA_KEYCODE_LIST \
  X(UNKNOWN,          Unknown,        SDLK_UNKNOWN)       \
  X(RETURN,           Return,         SDLK_RETURN)        \
  X(ESCAPE,           Escape,         SDLK_ESCAPE)        \
  X(BACKSPACE,        Backspace,      SDLK_BACKSPACE)     \
  X(TAB,              Tab,            SDLK_TAB)           \
  X(SPACE,            Space,          SDLK_SPACE)         \
  X(EXCLAIM,          Exclaim,        SDLK_EXCLAIM)       \
  X(QUOTE_DOUBLE,     QuoteDouble,    SDLK_QUOTEDBL)      \
  X(HASH,             Hash,           SDLK_HASH)          \
  X(PERCENT,          Percent,        SDLK_PERCENT)       \
  X(DOLLAR,           Dollar,         SDLK_DOLLAR)        \
  X(AMPERSAND,        Ampersand,      SDLK_AMPERSAND)     \
  X(QUOTE,            Quote,          SDLK_QUOTE)         \
  X(LEFT_PAREN,       LeftParen,      SDLK_LEFTPAREN)     \
  X(RIGHT_PAREN,      RightParen,     SDLK_RIGHTPAREN)    \
  X(ASTERISK,         Asterisk,       SDLK_ASTERISK)      \
  X(PLUS,             Plus,           SDLK_PLUS)          \
  X(COMMA,            Comma,          SDLK_COMMA)         \
  X(MINUS,            Minus,          SDLK_MINUS)         \
  X(PERIOD,           Period,         SDLK_PERIOD)        \
  X(SLASH,            Slash,          SDLK_SLASH)         \
  X(NUM0,             Num0,           SDLK_0)             \
  X(NUM1,             Num1,           SDLK_1)             \
  X(NUM2,             Num2,           SDLK_2)             \
  X(NUM3,             Num3,           SDLK_3)             \
  X(NUM4,             Num4,           SDLK_4)             \
  X(NUM5,             Num5,           SDLK_5)             \
  X(NUM6,             Num6,           SDLK_6)             \
  X(NUM7,             Num7,           SDLK_7)             \
  X(NUM8,             Num8,           SDLK_8)             \
  X(NUM9,             Num9,           SDLK_9)             \
  X(COLON,            Colon,          SDLK_COLON)         \
  X(SEMICOLON,        Semicolon,      SDLK_SEMICOLON)     \
  X(LESS,             Less,           SDLK_LESS)          \
  X(EQUALS,           Equals,         SDLK_EQUALS)        \
  X(GREATER,          Greater,        SDLK_GREATER)       \
  X(QUESTION,         Question,       SDLK_QUESTION)      \
  X(AT,               At,             SDLK_AT)            \
  X(LEFT_BRACKET,     LeftBracket,    SDLK_LEFTBRACKET)   \
  X(BACKSLASH,        Backslash,      SDLK_BACKSLASH)     \
  X(RIGHT_BRACKET,    RightBracket,   SDLK_RIGHTBRACKET)  \
  X(CARET,            Caret,          SDLK_CARET)         \
  X(UNDERSCORE,       Underscore,     SDLK_UNDERSCORE)    \
  X(BACKQUOTE,        Backquote,      SDLK_BACKQUOTE)     \
  X(A,                A,              SDLK_a)             \
  X(B,                B,              SDLK_b)             \
  X(C,                C,              SDLK_c)             \
  X(D,                D,              SDLK_d)             \
  X(E,                E,              SDLK_e)             \
  X(F,                F,              SDLK_f)             \
  X(G,                G,              SDLK_g)             \
  X(H,                H,              SDLK_h)             \
  X(I,                I,              SDLK_i)             \
  X(J,                J,              SDLK_j)             \
  X(K,                K,              SDLK_k)             \
  X(L,                L,              SDLK_l)             \
  X(M,                M,              SDLK_m)             \
  X(N,                N,              SDLK_n)             \
  X(O,                O,              SDLK_o)             \
  X(P,                P,              SDLK_p)             \
  X(Q,                Q,              SDLK_q)             \
  X(R,                R,              SDLK_r)             \
  X(S,                S,              SDLK_s)             \
  X(T,                T,              SDLK_t)             \
  X(U,                U,              SDLK_u)             \
  X(V,                V,              SDLK_v)             \
  X(W,                W,              SDLK_w)             \
  X(X,                X,              SDLK_x)             \
  X(Y,                Y,              SDLK_y)             \
  X(Z,                Z,              SDLK_z)             \
  X(CAPS_LOCK,        CapsLock,       SDLK_CAPSLOCK)      \
  X(F1,               F1,             SDLK_F1)            \
  X(F2,               F2,             SDLK_F2)            \
  X(F3,               F3,             SDLK_F3)            \
  X(F4,               F4,             SDLK_F4)            \
  X(F5,               F5,             SDLK_F5)            \
  X(F6,               F6,             SDLK_F6)            \
  X(F7,               F7,             SDLK_F7)            \
  X(F8,               F8,             SDLK_F8)            \
  X(F9,               F9,             SDLK_F9)            \
  X(F10,              F10,            SDLK_F10)           \
  X(F11,              F11,            SDLK_F11)           \
  X(F12,              F12,            SDLK_F12)           \
  X(PRINT_SCREEN,     PrintScreen,    SDLK_PRINTSCREEN)   \
  X(SCROLL_LOCK,      ScrollLock,     SDLK_SCROLLLOCK)    \
  X(PAUSE,            Pause,          SDLK_PAUSE)         \
  X(INSERT,           Insert,         SDLK_INSERT)        \
  X(HOME,             Home,           SDLK_HOME)          \
  X(PAGE_UP,          PageUp,         SDLK_PAGEUP)        \
  X(DELETE,           Delete,         SDLK_DELETE)        \
  X(END,              End,            SDLK_END)           \
  X(PAGE_DOWN,        PageDown,       SDLK_PAGEDOWN)      \
  X(RIGHT,            Right,          SDLK_RIGHT)         \
  X(LEFT,             Left,           SDLK_LEFT)          \
  X(DOWN,             Down,           SDLK_DOWN)          \
  X(UP,               Up,             SDLK_UP)            \
  X(NUM_LOCK,         NumLock,        SDLK_NUMLOCKCLEAR)  \
  X(NUMPAD_DIVIDE,    NumpadDivide,   SDLK_KP_DIVIDE)     \
  X(NUMPAD_MULTIPLY,  NumpadMultiply, SDLK_KP_MULTIPLY)   \
  X(NUMPAD_MINUS,     NumpadMinus,    SDLK_KP_MINUS)      \
  X(NUMPAD_PLUS,      NumpadPlus,     SDLK_KP_PLUS)       \
  X(NUMPAD_ENTER,     NumpadEnter,    SDLK_KP_ENTER)      \
  X(NUMPAD_1,         Numpad1,        SDLK_KP_1)          \
  X(NUMPAD_2,         Numpad2,        SDLK_KP_2)          \
  X(NUMPAD_3,         Numpad3,        SDLK_KP_3)          \
  X(NUMPAD_4,         Numpad4,        SDLK_KP_4)          \
  X(NUMPAD_5,         Numpad5,        SDLK_KP_5)          \
  X(NUMPAD_6,         Numpad6,        SDLK_KP_6)          \
  X(NUMPAD_7,         Numpad7,        SDLK_KP_7)          \
  X(NUMPAD_8,         Numpad8,        SDLK_KP_8)          \
  X(NUMPAD_9,         Numpad9,        SDLK_KP_9)          \
  X(NUMPAD_0,         Numpad0,        SDLK_KP_0)          \
  X(NUMPAD_PERIOD,    NumpadPeriod,   SDLK_KP_PERIOD)     \
  X(APPLICATION,      Application,    SDLK_APPLICATION)


#define X(name, fname, code)                      \
static void gammaKeycode ## fname(AgateVM *vm) {  \
  agateSlotSetInt(vm, AGATE_RETURN_SLOT, code);   \
}

GAMMA_KEYCODE_LIST

#undef X


/*
 * Keyboard
 */

// methods

static void gammaKeyboardScancodeName(AgateVM *vm) {
  int64_t scancode;

  if (!gammaCheckInt64(vm, 1, &scancode)) {
    gammaError(vm, "Int parameter expected for `scancode`.");
    agateSlotSetNil(vm, AGATE_RETURN_SLOT);
    return;
  }

  agateSlotSetString(vm, AGATE_RETURN_SLOT, SDL_GetScancodeName(scancode));
}

static void gammaKeyboardKeycodeName(AgateVM *vm) {
  int64_t keycode;

  if (!gammaCheckInt64(vm, 1, &keycode)) {
    gammaError(vm, "Int parameter expected for `keycode`.");
    agateSlotSetNil(vm, AGATE_RETURN_SLOT);
    return;
  }

  agateSlotSetString(vm, AGATE_RETURN_SLOT, SDL_GetKeyName((SDL_Keycode) keycode));
}

static void gammaKeyboardLocalize(AgateVM *vm) {
  int64_t scancode;

  if (!gammaCheckInt64(vm, 1, &scancode)) {
    gammaError(vm, "Int parameter expected for `scancode`.");
    agateSlotSetNil(vm, AGATE_RETURN_SLOT);
    return;
  }

  agateSlotSetInt(vm, AGATE_RETURN_SLOT, SDL_GetKeyFromScancode(scancode));
}

static void gammaKeyboardUnlocalize(AgateVM *vm) {
  int64_t keycode;

  if (!gammaCheckInt64(vm, 1, &keycode)) {
    gammaError(vm, "Int parameter expected for `keycode`.");
    agateSlotSetNil(vm, AGATE_RETURN_SLOT);
    return;
  }

  agateSlotSetInt(vm, AGATE_RETURN_SLOT, SDL_GetScancodeFromKey((SDL_Keycode) keycode));
}

/*
 * Modifier
 */

// NAME, FNAME, CODE
#define GAMMA_MODIFIER_LIST \
  X(NONE,   None,       KMOD_NONE)    \
  X(LSHIFT, LeftShift,  KMOD_LSHIFT)  \
  X(RSHIFT, RightShift, KMOD_RSHIFT)  \
  X(SHIFT,  Shift,      KMOD_SHIFT)   \
  X(LCTRL,  LeftCtrl,   KMOD_LCTRL)   \
  X(RCTRL,  RightCtrl,  KMOD_RCTRL)   \
  X(CTRL,   Ctrl,       KMOD_CTRL)    \
  X(LALT,   LeftAlt,    KMOD_LALT)    \
  X(RALT,   RightAlt,   KMOD_RALT)    \
  X(ALT,    Alt,        KMOD_ALT)     \
  X(LGUI,   LeftGui,    KMOD_LGUI)    \
  X(RGUI,   RightGui,   KMOD_RGUI)    \
  X(GUI,    Gui,        KMOD_GUI)     \
  X(NUM,    Num,        KMOD_NUM)     \
  X(CAPS,   Caps,       KMOD_CAPS)    \
  X(MODE,   Mode,       KMOD_MODE)


#define X(name, fname, code)                      \
static void gammaModifier ## fname(AgateVM *vm) { \
  agateSlotSetInt(vm, AGATE_RETURN_SLOT, code);   \
}

GAMMA_MODIFIER_LIST

#undef X

/*
 * unit configuration
 */

AgateForeignClassHandler gammaEventClassHandler(AgateVM *vm, const char *unit_name, const char *class_name) {
  assert(gammaEquals(unit_name, "gamma/event"));
  AgateForeignClassHandler handler = { NULL, NULL, NULL };

  if (gammaEquals(class_name, "__GenericEvent")) {
    handler.allocate = gammaEventAllocate;
    handler.tag = gammaEventTag;
    return handler;
  }

  if (gammaEquals(class_name, "KeyboardEvent")) {
    handler.allocate = gammaEventAllocate;
    handler.tag = gammaEventTag;
    return handler;
  }

  if (gammaEquals(class_name, "MouseMotionEvent")) {
    handler.allocate = gammaEventAllocate;
    handler.tag = gammaEventTag;
    return handler;
  }

  if (gammaEquals(class_name, "MouseButtonEvent")) {
    handler.allocate = gammaEventAllocate;
    handler.tag = gammaEventTag;
    return handler;
  }

  if (gammaEquals(class_name, "MouseWheelEvent")) {
    handler.allocate = gammaEventAllocate;
    handler.tag = gammaEventTag;
    return handler;
  }

  if (gammaEquals(class_name, "__GenericWindowEvent")) {
    handler.allocate = gammaEventAllocate;
    handler.tag = gammaEventTag;
    return handler;
  }

  if (gammaEquals(class_name, "ResizeEvent")) {
    handler.allocate = gammaEventAllocate;
    handler.tag = gammaEventTag;
    return handler;
  }

  return handler;

}

AgateForeignMethodFunc gammaEventMethodHandler(AgateVM *vm, const char *unit_name, const char *class_name, AgateForeignMethodKind kind, const char *signature) {
  assert(gammaEquals(unit_name, "gamma/event"));

  if (gammaEquals(class_name, "Event")) {
    if (kind == AGATE_FOREIGN_METHOD_CLASS) {
      if (gammaEquals(signature, "poll()")) { return gammaEventPoll; }

      #define X(name, fname) if (gammaEquals(signature, #name)) { return gammaEvent ## fname; }
      GAMMA_EVENT_TYPE_LIST
      #undef X
    } else {
      if (gammaEquals(signature, "type")) { return gammaEventType; }
    }
  }

  if (gammaEquals(class_name, "KeyboardEvent")) {
    if (kind == AGATE_FOREIGN_METHOD_INSTANCE) {
      if (gammaEquals(signature, "window_id")) { return gammaKeyboardEventWindowId; }
      if (gammaEquals(signature, "pressed")) { return gammaKeyboardEventPressed; }
      if (gammaEquals(signature, "repeat")) { return gammaKeyboardEventRepeat; }
      if (gammaEquals(signature, "scancode")) { return gammaKeyboardEventScancode; }
      if (gammaEquals(signature, "keycode")) { return gammaKeyboardEventKeycode; }
      if (gammaEquals(signature, "modifiers")) { return gammaKeyboardEventModifiers; }
    }
  }

  if (gammaEquals(class_name, "MouseMotionEvent")) {
    if (kind == AGATE_FOREIGN_METHOD_INSTANCE) {
      if (gammaEquals(signature, "window_id")) { return gammaMouseMotionEventWindowId; }
      if (gammaEquals(signature, "coordinates")) { return gammaMouseMotionEventCoordinates; }
      if (gammaEquals(signature, "motion")) { return gammaMouseMotionEventMotion; }
    }
  }

  if (gammaEquals(class_name, "MouseButtonEvent")) {
    if (kind == AGATE_FOREIGN_METHOD_INSTANCE) {
      if (gammaEquals(signature, "window_id")) { return gammaMouseButtonEventWindowId; }
      if (gammaEquals(signature, "button")) { return gammaMouseButtonEventButton; }
      if (gammaEquals(signature, "pressed")) { return gammaMouseButtonEventPressed; }
      if (gammaEquals(signature, "clicks")) { return gammaMouseButtonEventClicks; }
      if (gammaEquals(signature, "coordinates")) { return gammaMouseButtonEventCoordinates; }
    }
  }

  if (gammaEquals(class_name, "MouseWheelEvent")) {
    if (kind == AGATE_FOREIGN_METHOD_INSTANCE) {
      if (gammaEquals(signature, "window_id")) { return gammaMouseWheelEventWindowId; }
      if (gammaEquals(signature, "offset")) { return gammaMouseWheelEventOffset; }
    }
  }

  if (gammaEquals(class_name, "WindowEvent")) {
    if (kind == AGATE_FOREIGN_METHOD_INSTANCE) {
      if (gammaEquals(signature, "window_id")) { return gammaWindowEventWindowId; }
    }
  }

  if (gammaEquals(class_name, "ResizeEvent")) {
    if (kind == AGATE_FOREIGN_METHOD_INSTANCE) {
      if (gammaEquals(signature, "size")) { return gammaResizeEventSize; }
    }
  }

  if (gammaEquals(class_name, "Mouse")) {
    if (kind == AGATE_FOREIGN_METHOD_CLASS) {
      #define X(name, fname, type) if (gammaEquals(signature, #name)) { return gammaMouse ## fname; }
      GAMMA_MOUSE_BUTTON_LIST
      #undef X
    }
  }

  if (gammaEquals(class_name, "Scancode")) {
    if (kind == AGATE_FOREIGN_METHOD_CLASS) {
      #define X(name, fname, type) if (gammaEquals(signature, #name)) { return gammaScancode ## fname; }
      GAMMA_SCANCODE_LIST
      #undef X
    }
  }

  if (gammaEquals(class_name, "Keycode")) {
    if (kind == AGATE_FOREIGN_METHOD_CLASS) {
      #define X(name, fname, type) if (gammaEquals(signature, #name)) { return gammaKeycode ## fname; }
      GAMMA_KEYCODE_LIST
      #undef X
    }
  }

  if (gammaEquals(class_name, "Keyboard")) {
    if (kind == AGATE_FOREIGN_METHOD_CLASS) {
      if (gammaEquals(signature, "scancode_name(_)")) { return gammaKeyboardScancodeName; }
      if (gammaEquals(signature, "keycode_name(_)")) { return gammaKeyboardKeycodeName; }
      if (gammaEquals(signature, "localize(_)")) { return gammaKeyboardLocalize; }
      if (gammaEquals(signature, "unlocalize(_)")) { return gammaKeyboardUnlocalize; }
    }
  }

  if (gammaEquals(class_name, "Modifier")) {
    if (kind == AGATE_FOREIGN_METHOD_CLASS) {
      #define X(name, fname, type) if (gammaEquals(signature, #name)) { return gammaModifier ## fname; }
      GAMMA_MODIFIER_LIST
      #undef X
    }
  }

  return NULL;
}
