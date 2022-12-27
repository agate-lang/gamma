#include "gamma_math.h"

#include <cassert>
#include <cmath>
#include <limits>

#include <functional>

#include "gamma_agate.h"

namespace gma {

  /*
   * Generic Vec2 api
   */

  template<typename Class>
  struct Vec2Api : Class {
    using value_type = typename Class::type;
    using member_type = typename value_type::member_type;

    static void zero(AgateVM *vm) {
      assert(agateCheckTag<Class>(vm, 0));
      auto vec = agateSlotGet<Class>(vm, 0);
      *vec = gma::vec(member_type(0), member_type(0));
    }

    static void new2(AgateVM *vm) {
      assert(agateCheckTag<Class>(vm, 0));
      auto vec = agateSlotGet<Class>(vm, 0);

      if (agateCheck(vm, 1, vec->x)) {
        agateError(vm, "%s parameter expected for `x`.", type_name<member_type>);
        return;
      }

      if (!agateCheck(vm, 2, vec->y)) {
        agateError(vm, "%s parameter expected for `y`.", type_name<member_type>);
        return;
      }
    }

    static void get_x(AgateVM *vm) {
      assert(agateCheckTag<Class>(vm, 0));
      auto vec = agateSlotGet<Class>(vm, 0);
      agateSlotSet(vm, AGATE_RETURN_SLOT, vec->x);
    }

    static void set_x(AgateVM *vm) {
      assert(agateCheckTag<Class>(vm, 0));
      auto vec = agateSlotGet<Class>(vm, 0);

      if (!agateCheck(vm, 1, vec->x)) {
        agateError(vm, "%s parameter expected for `value`.", type_name<member_type>);
        return;
      }

      agateSlotCopy(vm, AGATE_RETURN_SLOT, 1);
    }

    static void get_y(AgateVM *vm) {
      assert(agateCheckTag<Class>(vm, 0));
      auto vec = agateSlotGet<Class>(vm, 0);
      agateSlotSet(vm, AGATE_RETURN_SLOT, vec->y);
    }

    static void set_y(AgateVM *vm) {
      assert(agateCheckTag<Class>(vm, 0));
      auto vec = agateSlotGet<Class>(vm, 0);

      if (!agateCheck(vm, 1, vec->y)) {
        agateError(vm, "%s parameter expected for `value`.", type_name<member_type>);
        return;
      }

      agateSlotCopy(vm, AGATE_RETURN_SLOT, 1);
    }

    static void plus(AgateVM *vm) {
      assert(agateCheckTag<Class>(vm, 0));
      auto vec = agateSlotGet<Class>(vm, 0);
      auto result = agateSlotNew<Class>(vm, AGATE_RETURN_SLOT);
      *result = *vec;
    }

    static void minus(AgateVM *vm) {
      assert(agateCheckTag<Class>(vm, 0));
      auto vec = agateSlotGet<Class>(vm, 0);
      auto result = agateSlotNew<Class>(vm, AGATE_RETURN_SLOT);
      *result = - *vec;
    }

    template<typename Op>
    static void binary_op(AgateVM *vm, Op op) {
      assert(agateCheckTag<Class>(vm, 0));
      auto vec = agateSlotGet<Class>(vm, 0);
      auto result = agateSlotNew<Class>(vm, AGATE_RETURN_SLOT);

      member_type value;

      if (agateCheck(vm, 1, value)) {
        *result = op(*vec, gma::vec(value, value));
        return;
      }

      value_type other;

      if (agateCheck(vm, 1, other)) {
        *result = op(*vec, other);
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
      assert(agateCheckTag<Class>(vm, 0));
      auto vec = agateSlotGet<Class>(vm, 0);

      value_type other;

      if (!agateCheck(vm, 1, other)) {
        agateError(vm, "%s parameter expected for `value`.", type_name<value_type>);
        return;
      }

      agateSlotSetBool(vm, AGATE_RETURN_SLOT, op(*vec, other));
    }

    static void eq(AgateVM *vm) { logical_op(vm, std::equal_to<>()); }
    static void neq(AgateVM *vm) { logical_op(vm, std::not_equal_to<>()); }

  };

  /*
   * Generic Vec2 check
   */

  template<typename Class>
  bool agateCheckVec(AgateVM *vm, ptrdiff_t slot, typename Class::type& result) {
    AgateType type = agateSlotType(vm, slot);

    if (type == AGATE_TYPE_FOREIGN) {
      uint64_t tag = agateSlotGetForeignTag(vm, slot);

      if (tag == Class::tag) {
        auto vec = agateSlotGet<Class>(vm, slot);
        result = *vec;
        return true;
      }

      return false;
    }

    if (type == AGATE_TYPE_TUPLE) {
      if (agateSlotTupleSize(vm, slot) != 2) {
        return false;
      }

      ptrdiff_t component_slot = agateSlotAllocate(vm);

      agateSlotTupleGet(vm, slot, 0, component_slot);
      if (!agateCheck(vm, component_slot, result.x)) { return false; }

      agateSlotTupleGet(vm, slot, 1, component_slot);
      if (!agateCheck(vm, component_slot, result.y)) { return false; }

      return true;
    }

    if (type == AGATE_TYPE_ARRAY) {
      if (agateSlotArraySize(vm, slot) != 2) {
        return false;
      }

      ptrdiff_t element_slot = agateSlotAllocate(vm);

      agateSlotArrayGet(vm, slot, 0, element_slot);
      if (!agateCheck(vm, element_slot, result.x)) { return false; }

      agateSlotArrayGet(vm, slot, 0, element_slot);
      if (!agateCheck(vm, element_slot, result.y)) { return false; }

      return true;
    }

    return false;
  }

  /*
   * Specialization for Vec2F
   */

  struct Vec2FApi : Vec2Api<Vec2FClass> {
    static void unit(AgateVM *vm) {
      assert(agateCheckTag<Vec2FClass>(vm, 0));
      auto vec = agateSlotGet<Vec2FClass>(vm, 0);

      float angle;

      if (agateCheck(vm, 1, angle)) {
        agateError(vm, "Float parameter expected for `angle`.");
        return;
      }

      *vec = { std::cos(angle), std::sin(angle) };
    }
  };

  bool agateCheck(AgateVM *vm, ptrdiff_t slot, Vec2F& result) {
    return agateCheckVec<Vec2FClass>(vm, slot, result);
  }

  /*
   * Specialization for Vec2I
   */

  using Vec2IApi = Vec2Api<Vec2IClass>;

  bool agateCheck(AgateVM *vm, ptrdiff_t slot, Vec2I& result) {
    return agateCheckVec<Vec2IClass>(vm, slot, result);
  }

  // =========================================================================

  /*
   * Generic Rect api
   */

  template<typename Class>
  struct RectApi : Class {
    using member_class = typename Class::member_class;

    using value_type = typename Class::type;
    using member_type = typename Class::type::member_type;
    using member_member_type = typename Class::type::member_type::member_type;

    static void new4(AgateVM *vm) {
      assert(agateCheckTag<Class>(vm, 0));
      auto rect = agateSlotGet<Class>(vm, 0);

      if (!agateCheck(vm, 1, rect->position.x)) {
        agateError(vm, "%s parameter expected for `x`.", type_name<member_member_type>);
        return;
      }

      if (!agateCheck(vm, 2, rect->position.y)) {
        agateError(vm, "%s parameter expected for `y`.", type_name<member_member_type>);
        return;
      }

      if (!agateCheck(vm, 3, rect->size.x)) {
        agateError(vm, "%s parameter expected for `w`.", type_name<member_member_type>);
        return;
      }

      if (!agateCheck(vm, 4, rect->size.y)) {
        agateError(vm, "%s parameter expected for `h`.", type_name<member_member_type>);
        return;
      }
    }

    static void new2(AgateVM *vm) {
      assert(agateCheckTag<Class>(vm, 0));
      auto rect = agateSlotGet<Class>(vm, 0);

      if (!agateCheck(vm, 1, rect->position)) {
        agateError(vm, "%s parameter expected for `position`.", type_name<member_type>);
        return;
      }

      if (!agateCheck(vm, 2, rect->size)) {
        agateError(vm, "%s parameter expected for `size`.", type_name<member_type>);
        return;
      }
    }

    static void new1(AgateVM *vm) {
      assert(agateCheckTag<Class>(vm, 0));
      auto rect = agateSlotGet<Class>(vm, 0);

      rect->position = gma::vec(member_member_type(0), member_member_type(0));

      if (!agateCheck(vm, 1, rect->size)) {
        agateError(vm, "%s parameter expected for `size`.", type_name<member_type>);
        return;
      }
    }

    static void generic_get(AgateVM *vm, member_type value_type::* pm, member_member_type member_type::* pmm) {
      assert(agateCheckTag<Class>(vm, 0));
      auto rect = agateSlotGet<Class>(vm, 0);
      agateSlotSet(vm, AGATE_RETURN_SLOT, rect->*pm.*pmm);
    }

    static void get_x(AgateVM *vm) { generic_get(vm, &value_type::position, &member_type::x); }
    static void get_y(AgateVM *vm) { generic_get(vm, &value_type::position, &member_type::y); }
    static void get_w(AgateVM *vm) { generic_get(vm, &value_type::size, &member_type::x); }
    static void get_h(AgateVM *vm) { generic_get(vm, &value_type::size, &member_type::y); }

    static void generic_set(AgateVM *vm, member_type value_type::* pm, member_member_type member_type::* pmm, const char *member_name) {
      assert(agateCheckTag<Class>(vm, 0));
      auto rect = agateSlotGet<Class>(vm, 0);

      if (!agateCheck(vm, 1, rect->*pm.*pmm)) {
        agateError(vm, "%s parameter expected for `%s`.", type_name<member_member_type>, member_name);
        return;
      }

      agateSlotCopy(vm, AGATE_RETURN_SLOT, 1);
    }

    static void set_x(AgateVM *vm) { generic_set(vm, &value_type::position, &member_type::x, "x"); }
    static void set_y(AgateVM *vm) { generic_set(vm, &value_type::position, &member_type::y, "y"); }
    static void set_w(AgateVM *vm) { generic_set(vm, &value_type::size, &member_type::x, "w"); }
    static void set_h(AgateVM *vm) { generic_set(vm, &value_type::size, &member_type::y, "h"); }

    static void generic_get(AgateVM *vm, member_type value_type::* pm) {
      assert(agateCheckTag<Class>(vm, 0));
      auto rect = agateSlotGet<Class>(vm, 0);

      auto result = agateSlotNew<member_class>(vm, AGATE_RETURN_SLOT);
      *result = rect->*pm;
    }

    static void get_position(AgateVM *vm) { generic_get(vm, &value_type::position); }
    static void get_size(AgateVM *vm) { generic_get(vm, &value_type::size); }

    static void generic_set(AgateVM *vm, member_type value_type::* pm, const char *member_name) {
      assert(agateCheckTag<Class>(vm, 0));
      auto rect = agateSlotGet<Class>(vm, 0);

      if (!agateCheck(vm, 1, rect->*pm)) {
        agateError(vm, "%s parameter expected for `%s`.", type_name<member_type>, member_name);
        return;
      }

      agateSlotCopy(vm, AGATE_RETURN_SLOT, 1);
    }

    static void set_position(AgateVM *vm) { generic_set(vm, &value_type::position, "position"); }
    static void set_size(AgateVM *vm) { generic_set(vm, &value_type::size, "size"); }

    static void contains(AgateVM *vm) {
      assert(agateCheckTag<Class>(vm, 0));
      auto rect = agateSlotGet<Class>(vm, 0);

      member_type vec;

      if (agateCheck(vm, 1, vec)) {
        agateSlotSet(vm, AGATE_RETURN_SLOT, rect->contains(vec));
        return;
      }

      value_type other;

      if (agateCheck(vm, 1, other)) {
        agateSlotSet(vm, AGATE_RETURN_SLOT, rect->contains(other));
        return;
      }

      agateError(vm, "%s or %s parameter expected for `other`.", type_name<value_type>, type_name<member_type>);
    }

    static void intersects(AgateVM *vm) {
      assert(agateCheckTag<Class>(vm, 0));
      auto rect = agateSlotGet<Class>(vm, 0);

      value_type other;

      if (agateCheck(vm, 1, other)) {
        agateSlotSet(vm, AGATE_RETURN_SLOT, rect->intersects(other));
        return;
      }

      agateError(vm, "%s parameter expected for `other`.", type_name<value_type>);
    }

  };

  /*
   * Generic Rect check
   */

  template<typename Class>
  bool agateCheckRect(AgateVM *vm, ptrdiff_t slot, typename Class::type& result) {
    AgateType type = agateSlotType(vm, slot);

    if (type == AGATE_TYPE_FOREIGN) {
      uint64_t tag = agateSlotGetForeignTag(vm, slot);

      if (tag == Class::tag) {
        auto rect = agateSlotGet<Class>(vm, slot);
        result = *rect;
        return true;
      }

      return false;
    }

    if (type == AGATE_TYPE_TUPLE) {
      ptrdiff_t component_count = agateSlotTupleSize(vm, slot);

      if (component_count == 2) {
        ptrdiff_t component_slot = agateSlotAllocate(vm);

        agateSlotTupleGet(vm, slot, 0, component_slot);

        if (!agateCheck(vm, component_slot, result.position)) {
          return false;
        }

        agateSlotTupleGet(vm, slot, 1, component_slot);

        if (!agateCheck(vm, component_slot, result.size)) {
          return false;
        }

        return true;
      }

      if (component_count == 4) {
        ptrdiff_t component_slot = agateSlotAllocate(vm);

        #define GAMMA_RECT_CHECK_TUPLE(field, i)              \
        agateSlotTupleGet(vm, slot, i, component_slot);       \
        if (!agateCheck(vm, component_slot, result.field)) {  \
          return false;                                       \
        }

        GAMMA_RECT_CHECK_TUPLE(position.x, 0)
        GAMMA_RECT_CHECK_TUPLE(position.y, 1)
        GAMMA_RECT_CHECK_TUPLE(size.x,     2)
        GAMMA_RECT_CHECK_TUPLE(size.y,     3)

        #undef GAMMA_RECT_CHECK_TUPLE

        return true;
      }

      return false;
    }

    if (type == AGATE_TYPE_ARRAY) {
      ptrdiff_t element_count = agateSlotArraySize(vm, slot);

      if (element_count == 2) {
        ptrdiff_t element_slot = agateSlotAllocate(vm);

        agateSlotArrayGet(vm, slot, 0, element_count);

        if (!agateCheck(vm, element_slot, result.position)) {
          return false;
        }

        agateSlotTupleGet(vm, slot, 1, element_slot);

        if (!agateCheck(vm, element_slot, result.size)) {
          return false;
        }

        return true;
      }

      if (element_count == 4) {
        ptrdiff_t element_slot = agateSlotAllocate(vm);

        #define GAMMA_RECT_CHECK_ARRAY(field, i)            \
        agateSlotArrayGet(vm, slot, i, element_slot);       \
        if (!agateCheck(vm, element_slot, result.field)) {  \
          return false;                                     \
        }

        GAMMA_RECT_CHECK_ARRAY(position.x, 0)
        GAMMA_RECT_CHECK_ARRAY(position.y, 1)
        GAMMA_RECT_CHECK_ARRAY(size.x,     2)
        GAMMA_RECT_CHECK_ARRAY(size.y,     3)

        #undef GAMMA_RECT_CHECK_ARRAY

        return true;
      }

      return false;
    }

    return false;
  }

  /*
   * Specialization for RectF
   */

  using RectFApi = RectApi<RectFClass>;

  bool agateCheck(AgateVM *vm, ptrdiff_t slot, RectF& result) {
    return agateCheckRect<RectFClass>(vm, slot, result);
  }

  /*
   * Specialization for RectI
   */

  using RectIApi = RectApi<RectIClass>;

  bool agateCheck(AgateVM *vm, ptrdiff_t slot, RectI& result) {
    return agateCheckRect<RectIClass>(vm, slot, result);
  }

  /*
   * Mat3F
   */

  Mat3F operator*(const Mat3F& lhs, const Mat3F& rhs) {
    Mat3F result;

    for (int col = 0; col < 3; ++col) {
      for (int row = 0; row < 3; ++row) {
        float sum = 0.0f;

        for (int k = 0; k < 3; ++k) {
          sum += lhs(row, k) * rhs(k, col);
        }

        result(row, col) = sum;
      }
    }

    return result;
  }

  Vec2F transform_point(const Mat3F& mat, Vec2F vec) {
    Vec2F result;
    result[0] = mat(0, 0) * vec[0] + mat(0, 1) * vec[1] + mat(0, 2);
    result[1] = mat(1, 0) * vec[0] + mat(1, 1) * vec[1] + mat(1, 2);
    return result;
  }

  Vec2F transform_vector(const Mat3F& mat, Vec2F vec) {
    Vec2F result;
    result[0] = mat(0, 0) * vec[0] + mat(0, 1) * vec[1];
    result[1] = mat(1, 0) * vec[0] + mat(1, 1) * vec[1];
    return result;
  }

  Mat3F inverse(const Mat3F& mat) {
    Mat3F result;
    result(0, 0) = mat(1, 1) * mat(2, 2) - mat(2, 1) * mat(1, 2);
    result(0, 1) = - (mat(0, 1) * mat(2, 2) - mat(2, 1) * mat(0, 2));
    result(0, 2) = mat(0, 1) * mat(1, 2) - mat(1, 1) * mat(0, 2);
    result(1, 0) = - (mat(1, 0) * mat(2, 2) - mat(2, 0) * mat(1, 2));
    result(1, 1) = mat(0, 0) * mat(2, 2) - mat(2, 0) * mat(0, 2);
    result(1, 2) = - (mat(0, 0) * mat(1, 2) - mat(1, 0) * mat(0, 2));
    result(2, 0) = mat(1, 0) * mat(2, 1) - mat(2, 0) * mat(1, 1);
    result(2, 1) = - (mat(0, 0) * mat(2, 1) - mat(2, 0) * mat(0, 1));
    result(2, 2) = mat(0, 0) * mat(1, 1) - mat(1, 0) * mat(0, 1);

    float det = mat(0, 0) * result(0, 0) + mat(0, 1) * result(1, 0) + mat(0, 2) * result(2, 0);
    assert(std::abs(det) > std::numeric_limits<float>::epsilon());

    for (int col = 0; col < 3; ++col) {
      for (int row = 0; row < 3; ++row) {
        result(row, col) /= det;
      }
    }

    return result;
  }

  Mat3F translation(Vec2F offset) {
    Mat3F mat;
    mat(0, 0) = 1.0; mat(0, 1) = 0.0; mat(0, 2) = offset[0];
    mat(1, 0) = 0.0; mat(1, 1) = 1.0; mat(1, 2) = offset[1];
    mat(2, 0) = 0.0; mat(2, 1) = 0.0; mat(2, 2) = 1.0;
    return mat;
  }


  /*
   * MathUnit
   */

  void MathUnit::provide_support(Support & support) {
    provide_vec_support<Vec2FApi>(support);
    provide_vec_support<Vec2IApi>(support);

    // special methods for Vec2F
    support.add_method(unit_name, Vec2FApi::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "init unit(_)", Vec2FApi::unit);

    provide_rect_support<RectFApi>(support);
    provide_rect_support<RectIApi>(support);
  }

  template<typename Api>
  void MathUnit::provide_vec_support(Support & support) {
    support.add_class_handler(unit_name, Api::class_name, generic_simple_handler<Api>());

    support.add_method(unit_name, Api::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "init zero()", Api::zero);
    support.add_method(unit_name, Api::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "init new(_,_)", Api::new2);
    support.add_method(unit_name, Api::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "x", Api::get_x);
    support.add_method(unit_name, Api::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "x=(_)", Api::set_x);
    support.add_method(unit_name, Api::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "y", Api::get_y);
    support.add_method(unit_name, Api::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "y=(_)", Api::set_y);
    support.add_method(unit_name, Api::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "+", Api::plus);
    support.add_method(unit_name, Api::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "-", Api::minus);
    support.add_method(unit_name, Api::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "+(_)", Api::add);
    support.add_method(unit_name, Api::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "-(_)", Api::sub);
    support.add_method(unit_name, Api::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "*(_)", Api::mul);
    support.add_method(unit_name, Api::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "/(_)", Api::div);
    support.add_method(unit_name, Api::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "==(_)", Api::eq);
    support.add_method(unit_name, Api::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "!=(_)", Api::neq);
  }

  template<typename Api>
  void MathUnit::provide_rect_support(Support & support) {
    support.add_class_handler(unit_name, Api::class_name, generic_simple_handler<Api>());

    support.add_method(unit_name, Api::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "init new(_,_,_,_)", Api::new4);
    support.add_method(unit_name, Api::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "init new(_,_)", Api::new2);
    support.add_method(unit_name, Api::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "init new(_)", Api::new1);
    support.add_method(unit_name, Api::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "x", Api::get_x);
    support.add_method(unit_name, Api::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "x=(_)", Api::set_x);
    support.add_method(unit_name, Api::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "y", Api::get_y);
    support.add_method(unit_name, Api::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "y=(_)", Api::set_y);
    support.add_method(unit_name, Api::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "w", Api::get_w);
    support.add_method(unit_name, Api::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "w=(_)", Api::set_w);
    support.add_method(unit_name, Api::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "h", Api::get_h);
    support.add_method(unit_name, Api::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "h=(_)", Api::set_h);
    support.add_method(unit_name, Api::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "position", Api::get_position);
    support.add_method(unit_name, Api::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "position=(_)", Api::set_position);
    support.add_method(unit_name, Api::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "size", Api::get_size);
    support.add_method(unit_name, Api::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "size=(_)", Api::set_size);
    support.add_method(unit_name, Api::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "contains(_)", Api::contains);
    support.add_method(unit_name, Api::class_name, AGATE_FOREIGN_METHOD_INSTANCE, "intersects(_)", Api::intersects);
  }

}
