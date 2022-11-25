#include "gamma_event.h"

#include <assert.h>

#include <SDL2/SDL.h>

#include "gamma_common.h"
#include "gamma_tags.h"

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
          gammaEventNewForeign(vm, "GenericEvent", event);
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
          gammaEventNewForeign(vm, "GenericEvent", event);
          return true;
        default:
          return false;
      }
      break;
    case SDL_QUIT:
      gammaEventNewForeign(vm, "GenericEvent", event);
      return true;
    case SDL_KEYDOWN:
    case SDL_KEYUP:
    case SDL_MOUSEWHEEL:
    case SDL_MOUSEBUTTONDOWN:
    case SDL_MOUSEBUTTONUP:
    case SDL_MOUSEMOTION:
    case SDL_CONTROLLERDEVICEADDED:
    case SDL_CONTROLLERDEVICEREMOVED:
    case SDL_CONTROLLERBUTTONDOWN:
    case SDL_CONTROLLERBUTTONUP:
    case SDL_CONTROLLERAXISMOTION:
    case SDL_TEXTINPUT:
    case SDL_FINGERDOWN:
    case SDL_FINGERMOTION:
    case SDL_FINGERUP:
      gammaEventNewForeign(vm, "GenericEvent", event);
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
 * Scancode
 */

// NAME, FNAME, CODE
#define GAMMA_SCANCODE_LIST \
  X(UNKNOWN,  Unknown,  SDL_SCANCODE_UNKNOWN) \
  X(NUM0,     Num0,     SDL_SCANCODE_0)       \
  X(NUM1,     Num1,     SDL_SCANCODE_1)

#define X(name, fname, code)                      \
static void gammaScancode ## fname(AgateVM *vm) { \
  agateSlotSetInt(vm, AGATE_RETURN_SLOT, code);   \
}

GAMMA_SCANCODE_LIST

#undef X


/*
 * unit configuration
 */

AgateForeignClassHandler gammaEventClassHandler(AgateVM *vm, const char *unit_name, const char *class_name) {
  assert(gammaEquals(unit_name, "gamma/event"));
  AgateForeignClassHandler handler = { NULL, NULL, NULL };

  if (gammaEquals(class_name, "GenericEvent")) {
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

  if (gammaEquals(class_name, "Scancode")) {
    if (kind == AGATE_FOREIGN_METHOD_CLASS) {
      #define X(name, fname, type) if (gammaEquals(signature, #name)) { return gammaScancode ## fname; }
      GAMMA_SCANCODE_LIST
      #undef X
    }
  }

  return NULL;
}
