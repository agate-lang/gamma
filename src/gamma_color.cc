#include "gamma_color.h"

#include <cassert>
#include <cmath>
#include <algorithm>
#include <functional>
#include <limits>

#include "gamma_agate.h"

namespace gma {

  /*
   * Color
   */

  namespace {

    struct HSV {
      float h;
      float s;
      float v;
      float a;
    };

    static void convert_rgb_to_hsv(HSV & hsv, const Color & color) {
      const float r = color.r;
      const float g = color.g;
      const float b = color.b;

      const auto [ min, max ] = std::minmax({ r, g, b });

      if ((max - min) > std::numeric_limits<float>::epsilon()) {
        if (max == r) {
          hsv.h = std::fmod(60.0f * (g - b) / (max - min) + 360.0f, 360.0f);
        } else if (max == g) {
          hsv.h = 60.0f * (b - r) / (max - min) + 120.0f;
        } else if (max == b) {
          hsv.h = 60.0f * (r - g) / (max - min) + 240.0f;
        } else {
          assert(false);
        }
      } else {
        hsv.h = 0.0f;
      }

      hsv.s = (max < std::numeric_limits<float>::epsilon() ? 0.0f : (1.0f - min / max));
      hsv.v = max;
      hsv.a = color.a;
    }

    static void convert_hsv_to_rgb(Color & color, const HSV & hsv) {
      const float h = hsv.h / 60.0f;
      const float s = hsv.s;
      const float v = hsv.v;

      const int i = static_cast<int>(h) % 6;
      assert(0 <= i && i < 6);

      const float f = h - static_cast<float>(i);
      const float x = v * (1.0f - s);
      const float y = v * (1.0f - (f * s));
      const float z = v * (1.0f - (1.0f - f) * s);

      switch (i) {
        case 0: color.r = v; color.g = z; color.b = x; break;
        case 1: color.r = y; color.g = v; color.b = x; break;
        case 2: color.r = x; color.g = v; color.b = z; break;
        case 3: color.r = x; color.g = y; color.b = v; break;
        case 4: color.r = z; color.g = x; color.b = v; break;
        case 5: color.r = v; color.g = x; color.b = y; break;
        default: assert(false); break;
      }

      color.a = hsv.a;
    }

  }

  Color Color::darker(float ratio) const {
    HSV hsv;
    convert_rgb_to_hsv(hsv, *this);

    hsv.v -= hsv.v * ratio;

    Color result;
    convert_hsv_to_rgb(result, hsv);
    return result;
  }

  Color Color::lighter(float ratio) const {
    HSV hsv;
    convert_rgb_to_hsv(hsv, *this);

    hsv.v += hsv.v * ratio;

    if (hsv.v > 1) {
      hsv.s -= (hsv.v - 1);

      if (hsv.s < 0) {
        hsv.s = 0;
      }

      hsv.v = 1;
    }

    Color result;
    convert_hsv_to_rgb(result, hsv);
    return result;
  }

  Color Color::rgba32(int64_t rgba) {
    Color color;
    color.r = ((rgba >> 24) & 0xFF) / 255.0f;
    color.g = ((rgba >> 16) & 0xFF) / 255.0f;
    color.b = ((rgba >>  8) & 0xFF) / 255.0f;
    color.a = ((rgba >>  0) & 0xFF) / 255.0f;
    return color;
  }

  Color Color::rgb24(int64_t rgb) {
    Color color;
    color.r = ((rgb >> 16) & 0xFF) / 255.0f;
    color.g = ((rgb >>  8) & 0xFF) / 255.0f;
    color.b = ((rgb >>  0) & 0xFF) / 255.0f;
    color.a = 1.0f;
    return color;
  }

  /*
   * Color api
   */

  struct ColorApi : ColorClass {
    using value_type = typename ColorClass::type;
    using member_type = typename ColorClass::type::member_type;

    static void new4(AgateVM *vm) {
      assert(agateCheckTag<ColorClass>(vm, 0));
      auto color = agateSlotGet<ColorClass>(vm, 0);

      if (!agateCheck(vm, 1, color->r)) {
        agateError(vm, "Float parameter expected for `r`.");
        return;
      }

      if (!agateCheck(vm, 2, color->g)) {
        agateError(vm, "Float parameter expected for `g`.");
        return;
      }

      if (!agateCheck(vm, 3, color->b)) {
        agateError(vm, "Float parameter expected for `b`.");
        return;
      }

      if (!agateCheck(vm, 4, color->a)) {
        agateError(vm, "Float parameter expected for `a`.");
        return;
      }

      color->r = std::clamp(color->r, 0.0f, 1.0f);
      color->g = std::clamp(color->g, 0.0f, 1.0f);
      color->b = std::clamp(color->b, 0.0f, 1.0f);
      color->a = std::clamp(color->a, 0.0f, 1.0f);
    }

    static void rgba32(AgateVM *vm) {
      assert(agateCheckTag<ColorClass>(vm, 0));
      auto color = agateSlotGet<ColorClass>(vm, 0);

      int64_t rgba;

      if (!agateCheck(vm, 1, rgba)) {
        agateError(vm, "Int parameter expected for `value`.");
        return;
      }

      *color = Color::rgba32(rgba);
    }

    static void rgb24(AgateVM *vm) {
      assert(agateCheckTag<ColorClass>(vm, 0));
      auto color = agateSlotGet<ColorClass>(vm, 0);

      int64_t rgb;

      if (!agateCheck(vm, 1, rgb)) {
        agateError(vm, "Int parameter expected for `value`.");
        return;
      }

      *color = Color::rgb24(rgb);
    }

    static void generic_get(AgateVM *vm, member_type value_type::* pm) {
      assert(agateCheckTag<ColorClass>(vm, 0));
      auto color = agateSlotGet<ColorClass>(vm, 0);
      agateSlotSet(vm, AGATE_RETURN_SLOT, color->*pm);
    }

    static void get_r(AgateVM *vm) { generic_get(vm, &value_type::r); }
    static void get_g(AgateVM *vm) { generic_get(vm, &value_type::g); }
    static void get_b(AgateVM *vm) { generic_get(vm, &value_type::b); }
    static void get_a(AgateVM *vm) { generic_get(vm, &value_type::a); }

    static void generic_set(AgateVM *vm, member_type value_type::* pm, const char *member_name) {
      assert(agateCheckTag<ColorClass>(vm, 0));
      auto color = agateSlotGet<ColorClass>(vm, 0);

      if (!agateCheck(vm, 1, color->*pm)) {
        agateError(vm, "%s parameter expected for `%s`.", type_name<member_type>, member_name);
        return;
      }

      agateSlotCopy(vm, AGATE_RETURN_SLOT, 1);
    }

    static void set_r(AgateVM *vm) { generic_set(vm, &value_type::r, "r"); }
    static void set_g(AgateVM *vm) { generic_set(vm, &value_type::g, "g"); }
    static void set_b(AgateVM *vm) { generic_set(vm, &value_type::b, "b"); }
    static void set_a(AgateVM *vm) { generic_set(vm, &value_type::a, "a"); }

    template<typename Op>
    static void binary_op(AgateVM *vm, Op op) {
      assert(agateCheckTag<ColorClass>(vm, 0));
      auto color = agateSlotGet<ColorClass>(vm, 0);
      auto result = agateSlotNew<ColorClass>(vm, AGATE_RETURN_SLOT);

      member_type value;

      if (agateCheck(vm, 1, value)) {
        Color other = { value, value, value, value };
        *result = op(*color, other);
        return;
      }

      value_type other;

      if (agateCheck(vm, 1, other)) {
        *result = op(*color, other);
        return;
      }

      agateError(vm, "%s or %s parameter expected for `other`.", type_name<value_type>, type_name<member_type>);
    }

    static void add(AgateVM *vm) { binary_op(vm, std::plus<>()); }
    static void sub(AgateVM *vm) { binary_op(vm, std::minus<>()); }
    static void mul(AgateVM *vm) { binary_op(vm, std::multiplies<>()); }
    static void div(AgateVM *vm) { binary_op(vm, std::divides<>()); }

    template<typename Op>
    static void logical_op(AgateVM *vm, Op op) {
      assert(agateCheckTag<ColorClass>(vm, 0));
      auto color = agateSlotGet<ColorClass>(vm, 0);

      value_type other;

      if (!agateCheck(vm, 1, other)) {
        agateError(vm, "%s parameter expected for `value`.", type_name<value_type>);
        return;
      }

      agateSlotSetBool(vm, AGATE_RETURN_SLOT, op(*color, other));
    }

    static void eq(AgateVM *vm) { logical_op(vm, std::equal_to<>()); }
    static void neq(AgateVM *vm) { logical_op(vm, std::not_equal_to<>()); }

    static void darker(AgateVM *vm) {
      assert(agateCheckTag<ColorClass>(vm, 0));
      auto color = agateSlotGet<ColorClass>(vm, 0);

      float ratio;

      if (!agateCheck(vm, 1, ratio)) {
        agateError(vm, "Float parameter expected for `ratio`.");
        return;
      }

      auto result = agateSlotNew<ColorClass>(vm, AGATE_RETURN_SLOT);
      *result = color->darker(ratio);
    }

    static void lighter(AgateVM *vm) {
      assert(agateCheckTag<ColorClass>(vm, 0));
      auto color = agateSlotGet<ColorClass>(vm, 0);

      float ratio;

      if (!agateCheck(vm, 1, ratio)) {
        agateError(vm, "Float parameter expected for `ratio`.");
        return;
      }

      auto result = agateSlotNew<ColorClass>(vm, AGATE_RETURN_SLOT);
      *result = color->lighter(ratio);
    }

    static void normalize(AgateVM *vm) {
      assert(agateCheckTag<ColorClass>(vm, 0));
      auto color = agateSlotGet<ColorClass>(vm, 0);

      color->r = std::clamp(color->r, 0.0f, 1.0f);
      color->g = std::clamp(color->g, 0.0f, 1.0f);
      color->b = std::clamp(color->b, 0.0f, 1.0f);
      color->a = std::clamp(color->a, 0.0f, 1.0f);
    }

    static void lerp(AgateVM *vm) {
      Color color0;

      if (agateCheck(vm, 1, color0)) {
        agateError(vm, "Color parameter expected for `color0`.");
        return;
      }

      Color color1;

      if (agateCheck(vm, 2, color1)) {
        agateError(vm, "Color parameter expected for `color1`.");
        return;
      }

      float ratio;

      if (!agateCheck(vm, 3, ratio)) {
        agateError(vm, "Float parameter expected for `ratio`.");
        return;
      }

      auto result = agateSlotNew<ColorClass>(vm, AGATE_RETURN_SLOT);
      result->r = color0.r * (1.0f - ratio) + color1.r * ratio;
      result->g = color0.g * (1.0f - ratio) + color1.g * ratio;
      result->b = color0.b * (1.0f - ratio) + color1.b * ratio;
      result->a = color0.a * (1.0f - ratio) + color1.a * ratio;
    }

  };

  bool agateCheck(AgateVM *vm, ptrdiff_t slot, Color& result) {
    AgateType type = agateSlotType(vm, slot);

    if (type == AGATE_TYPE_FOREIGN) {
      uint64_t tag = agateSlotGetForeignTag(vm, slot);

      if (tag == ColorClass::tag) {
        auto color = agateSlotGet<ColorClass>(vm, slot);
        result = *color;
        return true;
      }

      return false;
    }

    if (type == AGATE_TYPE_INT) {
      int64_t rgba = agateSlotGetInt(vm, slot);
      result = Color::rgba32(rgba);
      return false;
    }

    if (type == AGATE_TYPE_TUPLE) {
      ptrdiff_t component_count = agateSlotTupleSize(vm, slot);

      if (component_count != 3 && component_count != 4) {
        return false;
      }

      ptrdiff_t component_slot = agateSlotAllocate(vm);

      #define GAMMA_COLOR_CHECK_TUPLE(field, i)             \
      agateSlotTupleGet(vm, slot, i, component_slot);       \
      if (!agateCheck(vm, component_slot, result.field)) {  \
        return false;                                       \
      }

      GAMMA_COLOR_CHECK_TUPLE(r, 0)
      GAMMA_COLOR_CHECK_TUPLE(g, 1)
      GAMMA_COLOR_CHECK_TUPLE(b, 2)

      if (component_count == 4) {
        GAMMA_COLOR_CHECK_TUPLE(a, 3)
      } else {
        result.a = 1.0f;
      }

      #undef GAMMA_COLOR_CHECK_TUPLE

      return true;
    }

    if (type == AGATE_TYPE_ARRAY) {
      ptrdiff_t element_count = agateSlotArraySize(vm, slot);

      if (element_count != 3 && element_count != 4) {
        return false;
      }

      ptrdiff_t element_slot = agateSlotAllocate(vm);

      #define GAMMA_COLOR_CHECK_ARRAY(field, i)           \
      agateSlotArrayGet(vm, slot, i, element_slot);       \
      if (!agateCheck(vm, element_slot, result.field)) {  \
        return false;                                     \
      }

      GAMMA_COLOR_CHECK_ARRAY(r, 0)
      GAMMA_COLOR_CHECK_ARRAY(g, 1)
      GAMMA_COLOR_CHECK_ARRAY(b, 2)

      if (element_count == 4) {
        GAMMA_COLOR_CHECK_ARRAY(a, 3)
      } else {
        result.a = 1.0f;
      }

      #undef GAMMA_COLOR_CHECK_ARRAY

      return true;
    }

    return false;
  }


  /*
   * ColorUnit
   */

  void ColorUnit::provide_support(Support & support) {
    support.add_class_handler(unit_name, ColorClass::class_name, generic_simple_handler<ColorClass>());

    support.add_method(unit_name, ColorApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "init new(_,_,_,_)", ColorApi::new4);
    support.add_method(unit_name, ColorApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "init rgba32(_)", ColorApi::rgba32);
    support.add_method(unit_name, ColorApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "init rgb24(_)", ColorApi::rgb24);
    support.add_method(unit_name, ColorApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "r", ColorApi::get_r);
    support.add_method(unit_name, ColorApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "r=(_)", ColorApi::set_r);
    support.add_method(unit_name, ColorApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "g", ColorApi::get_g);
    support.add_method(unit_name, ColorApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "g=(_)", ColorApi::set_g);
    support.add_method(unit_name, ColorApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "b", ColorApi::get_b);
    support.add_method(unit_name, ColorApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "b=(_)", ColorApi::set_b);
    support.add_method(unit_name, ColorApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "a", ColorApi::get_a);
    support.add_method(unit_name, ColorApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "a=(_)", ColorApi::set_a);
    support.add_method(unit_name, ColorApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "+(_)", ColorApi::add);
    support.add_method(unit_name, ColorApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "-(_)", ColorApi::sub);
    support.add_method(unit_name, ColorApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "*(_)", ColorApi::mul);
    support.add_method(unit_name, ColorApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "/(_)", ColorApi::div);
    support.add_method(unit_name, ColorApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "==(_)", ColorApi::eq);
    support.add_method(unit_name, ColorApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "!=(_)", ColorApi::neq);
    support.add_method(unit_name, ColorApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "darker(_)", ColorApi::darker);
    support.add_method(unit_name, ColorApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "lighter(_)", ColorApi::lighter);
    support.add_method(unit_name, ColorApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "normalize()", ColorApi::normalize);
    support.add_method(unit_name, ColorApi::class_name, AGATE_FOREIGN_METHOD_CLASS, "lerp(_,_,_)", ColorApi::lerp);
  }

}
