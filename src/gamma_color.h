#ifndef GAMMA_COLOR_H
#define GAMMA_COLOR_H

#include "gamma_meta.h"
#include "gamma_support.h"

namespace gma {

  struct ColorUnit {
    static constexpr const char * unit_name = "gamma/color";
    static void provide_support(Support & support);
  };

  struct Color {
    using member_type = float;

    float r;
    float g;
    float b;
    float a;

    Color darker(float ratio) const;
    Color lighter(float ratio) const;

    static Color rgba32(int64_t rgba);
    static Color rgb24(int64_t rgb);
  };

  inline
  Color operator+(Color lhs, Color rhs) {
    return { lhs.r + rhs.r, lhs.g + rhs.g, lhs.b + rhs.b, lhs.a + rhs.a };
  }

  inline
  Color operator-(Color lhs, Color rhs) {
    return { lhs.r - rhs.r, lhs.g - rhs.g, lhs.b - rhs.b, lhs.a - rhs.a };
  }

  inline
  Color operator*(Color lhs, Color rhs) {
    return { lhs.r * rhs.r, lhs.g * rhs.g, lhs.b * rhs.b, lhs.a * rhs.a };
  }

  inline
  Color operator/(Color lhs, Color rhs) {
    return { lhs.r / rhs.r, lhs.g / rhs.g, lhs.b / rhs.b, lhs.a / rhs.a };
  }

  inline
  bool operator==(Color lhs, Color rhs) {
    return lhs.r == rhs.r && lhs.g == rhs.g && lhs.b == rhs.b && lhs.a == rhs.a;
  }

  inline
  bool operator!=(Color lhs, Color rhs) {
    return lhs.r != rhs.r || lhs.g != rhs.g || lhs.b != rhs.b || lhs.a != rhs.a;
  }

  template<>
  struct TypeName<Color> {
    static constexpr const char *name = "Color";
  };

  struct ColorClass : ColorUnit {
    using type = Color;
    static constexpr const char * class_name = type_name<type>;
    static constexpr uint64_t tag = compute_tag(unit_name, class_name);
  };

  bool agateCheck(AgateVM *vm, ptrdiff_t slot, Color& result);

}

#endif // GAMMA_COLOR_H
