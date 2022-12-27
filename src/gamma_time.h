#ifndef GAMMA_TIME_H
#define GAMMA_TIME_H

#include "gamma_support.h"

namespace gma {

  struct TimeUnit {
    static constexpr const char * unit_name = "gamma/time";
    static void provide_support(Support & support);
  };

  /*
   * Time
   */

  struct TimeClass : TimeUnit {
    static constexpr const char * class_name = "Time";
    static constexpr uint64_t tag = compute_tag(unit_name, class_name);
  };

}

#endif // GAMMA_TIME_H
