#include "gamma_event.h"

#include <assert.h>

#include <SDL2/SDL.h>

#include "gamma_common.h"

/*
 * Event
 */

struct gammaEvent {
  SDL_Event data;
};

//  NAME,                   FNAME,                TYPE
#define GAMMA_EVENT_TYPE_LIST \
  X(KEY_PRESSED,            KeyPressed,           SDL_KEYDOWN)          \
  X(KEY_RELEASED,           KeyReleased,          SDL_KEYUP)            \
  X(MOUSE_BUTTON_PRESSED,   MouseButtonPressed,   SDL_MOUSEBUTTONDOWN)  \
  X(MOUSE_BUTTON_RELEASED,  MouseButtonReleased,  SDL_MOUSEBUTTONUP)


#define X(name, fname, type)                    \
static void gammaEvent ## fname(AgateVM *vm) {  \
  agateSlotSetInt(vm, AGATE_RETURN_SLOT, type); \
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

AgateForeignMethodFunc gammaEventMethodHandler(AgateVM *vm, const char *unit_name, const char *class_name, AgateForeignMethodKind kind, const char *signature) {
  assert(gammaEquals(unit_name, "gamma/event"));

  if (gammaEquals(class_name, "Event")) {
    if (kind == AGATE_FOREIGN_METHOD_CLASS) {
      #define X(name, fname, type) if (gammaEquals(signature, #name)) { return gammaEvent ## fname; }
      GAMMA_EVENT_TYPE_LIST
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

  return NULL;
}
