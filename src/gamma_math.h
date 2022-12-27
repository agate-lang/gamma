#ifndef GAMMA_MATH_H
#define GAMMA_MATH_H

#include <cassert>

#include "gamma_meta.h"
#include "gamma_support.h"

namespace gma {

  struct MathUnit {
    static constexpr const char * unit_name = "gamma/math";
    static void provide_support(Support & support);

  private:
    template<typename Api>
    static void provide_vec_support(Support & support);
    template<typename Api>
    static void provide_rect_support(Support & support);
  };

  /*
   * Generic Vec2
   */

  template<typename T>
  struct Vec2 {
    using member_type = T;
    T x;
    T y;

    T operator[](int index) const {
      switch (index) {
        case 0: return x;
        case 1: return y;
        default: break;
      }
      assert(false);
    }

    T& operator[](int index) {
      switch (index) {
        case 0: return x;
        case 1: return y;
        default: break;
      }
      assert(false);
    }

  };

  template<typename T>
  inline
  Vec2<T> vec(T x, T y) {
    return { x, y };
  }

  template<typename T>
  inline
  Vec2<T> operator-(Vec2<T> rhs) {
    return { -rhs.x, -rhs.y };
  }

  template<typename U, typename V>
  inline
  Vec2<std::common_type_t<U, V>> operator+(Vec2<U> lhs, Vec2<V> rhs) {
    return { lhs.x + rhs.x, lhs.y + rhs.y };
  }

  template<typename U, typename V>
  inline
  Vec2<U>& operator+=(Vec2<U>& lhs, Vec2<V> rhs) {
    lhs.x += rhs.x;
    lhs.y += rhs.y;
    return lhs;
  }

  template<typename U, typename V, typename E = typename std::enable_if_t<std::is_arithmetic_v<U>, U>>
  inline
  Vec2<std::common_type_t<U, V>> operator+(Vec2<U> lhs, V rhs) {
    return { lhs.x + rhs, lhs.y + rhs };
  }

  template<typename U, typename V, typename E = typename std::enable_if_t<std::is_arithmetic_v<U>, U>>
  inline
  Vec2<U>& operator+=(Vec2<U>& lhs, V rhs) {
    lhs.x += rhs;
    lhs.y += rhs;
    return lhs;
  }

  template<typename U, typename V>
  inline
  Vec2<std::common_type_t<U, V>> operator-(Vec2<U> lhs, Vec2<V> rhs) {
    return { lhs.x - rhs.x, lhs.y - rhs.y };
  }

  template<typename U, typename V>
  inline
  Vec2<U>& operator-=(Vec2<U>& lhs, Vec2<V> rhs) {
    lhs.x -= rhs.x;
    lhs.y -= rhs.y;
    return lhs;
  }

  template<typename U, typename V, typename E = typename std::enable_if_t<std::is_arithmetic_v<U>, U>>
  inline
  Vec2<std::common_type_t<U, V>> operator-(Vec2<U> lhs, V rhs) {
    return { lhs.x - rhs, lhs.y - rhs };
  }

  template<typename U, typename V, typename E = typename std::enable_if_t<std::is_arithmetic_v<U>, U>>
  inline
  Vec2<U>& operator-=(Vec2<U>& lhs, V rhs) {
    lhs.x -= rhs;
    lhs.y -= rhs;
    return lhs;
  }

  template<typename U, typename V>
  inline
  Vec2<std::common_type_t<U, V>> operator*(Vec2<U> lhs, Vec2<V> rhs) {
    return { lhs.x * rhs.x, lhs.y * rhs.y };
  }

  template<typename U, typename V>
  inline
  Vec2<U>& operator*=(Vec2<U>& lhs, Vec2<V> rhs) {
    lhs.x *= rhs.x;
    lhs.y *= rhs.y;
    return lhs;
  }

  template<typename U, typename V, typename E = typename std::enable_if_t<std::is_arithmetic_v<U>, U>>
  inline
  Vec2<std::common_type_t<U, V>> operator*(Vec2<U> lhs, V rhs) {
    return { lhs.x * rhs, lhs.y * rhs };
  }

  template<typename U, typename V, typename E = typename std::enable_if_t<std::is_arithmetic_v<U>, U>>
  inline
  Vec2<U>& operator*=(Vec2<U>& lhs, V rhs) {
    lhs.x *= rhs;
    lhs.y *= rhs;
    return lhs;
  }

  template<typename U, typename V>
  inline
  Vec2<std::common_type_t<U, V>> operator/(Vec2<U> lhs, Vec2<V> rhs) {
    return { lhs.x / rhs.x, lhs.y / rhs.y };
  }

  template<typename U, typename V>
  inline
  Vec2<U>& operator/=(Vec2<U>& lhs, Vec2<V> rhs) {
    lhs.x /= rhs.x;
    lhs.y /= rhs.y;
    return lhs;
  }

  template<typename U, typename V, typename E = typename std::enable_if_t<std::is_arithmetic_v<U>, U>>
  inline
  Vec2<std::common_type_t<U, V>> operator/(Vec2<U> lhs, V rhs) {
    return { lhs.x / rhs, lhs.y / rhs };
  }

  template<typename U, typename V, typename E = typename std::enable_if_t<std::is_arithmetic_v<U>, U>>
  inline
  Vec2<U>& operator/=(Vec2<U>& lhs, V rhs) {
    lhs.x /= rhs;
    lhs.y /= rhs;
    return lhs;
  }

  template<typename T>
  inline
  bool operator==(Vec2<T> lhs, Vec2<T> rhs) {
    return lhs.x == rhs.x && lhs.y == rhs.y;
  }

  template<typename T>
  inline
  bool operator!=(Vec2<T> lhs, Vec2<T> rhs) {
    return lhs.x != rhs.x || lhs.y != rhs.y;
  }


  /*
   * Vec2F
   */

  using Vec2F = Vec2<float>;

  template<>
  struct TypeName<Vec2F> {
    static constexpr const char *name = "Vec2F";
  };

  struct Vec2FClass : MathUnit {
    using type = Vec2F;
    static constexpr const char * class_name = type_name<type>;
    static constexpr uint64_t tag = compute_tag(unit_name, class_name);
  };

  bool agateCheck(AgateVM *vm, ptrdiff_t slot, Vec2F& result);

  /*
   * Vec2I
   */

  using Vec2I = Vec2<int>;

  template<>
  struct TypeName<Vec2I> {
    static constexpr const char *name = "Vec2I";
  };

  struct Vec2IClass : MathUnit {
    using type = Vec2I;
    static constexpr const char * class_name = type_name<type>;
    static constexpr uint64_t tag = compute_tag(unit_name, class_name);
  };

  bool agateCheck(AgateVM *vm, ptrdiff_t slot, Vec2I& result);


  /*
   * Generic Rect
   */

  template<typename T>
  struct Rect {
    using member_type = Vec2<T>;
    Vec2<T> position;
    Vec2<T> size;

    bool contains(Vec2<T> other) const {
      return position.x <= other.x && other.x < position.x + size.x
          && position.y <= other.y && other.y < position.y + size.y;
    }

    bool contains(Rect other) const {
      return position.x <= other.position.x && other.position.x + other.size.x <= position.x + size.x
          && position.y <= other.position.y && other.position.y + other.size.y <= position.y + size.y;
    }

    bool intersects(Rect other) const {
      return position.x < other.position.x + other.size.x && other.position.x < position.x + size.x
          && position.y < other.position.y + other.size.y && other.position.y < position.y + size.y;
    }

  };

  /*
   * RectF
   */

  using RectF = Rect<float>;

  inline
  Vec2F compute_position(RectF rect, Vec2F uv) {
    return rect.position + uv * rect.size;
  }

  inline
  Vec2F compute_texture_position(RectF rect, Vec2F uv) {
    uv.y = 1.0f - uv.y;
    return compute_position(rect, uv);
  }

  template<>
  struct TypeName<RectF> {
    static constexpr const char *name = "RectF";
  };

  struct RectFClass : MathUnit {
    using type = RectF;
    using member_class = Vec2FClass;
    static constexpr const char * class_name = type_name<type>;
    static constexpr uint64_t tag = compute_tag(unit_name, class_name);
  };

  bool agateCheck(AgateVM *vm, ptrdiff_t slot, RectF& result);

  /*
   * RectI
   */

  using RectI = Rect<int>;

  template<>
  struct TypeName<RectI> {
    static constexpr const char *name = "RectI";
  };

  struct RectIClass : MathUnit {
    using type = RectI;
    using member_class = Vec2IClass;
    static constexpr const char * class_name = type_name<type>;
    static constexpr uint64_t tag = compute_tag(unit_name, class_name);
  };

  bool agateCheck(AgateVM *vm, ptrdiff_t slot, RectI& result);

  /*
   * Mat3F (not in the gamma API)
   */

  struct Mat3F {
    float m[3][3]; // column-major

    float operator()(int row, int col) const {
      assert(0 <= row && row < 3);
      assert(0 <= col && col < 3);
      return m[col][row];
    }

    float& operator()(int row, int col) {
      assert(0 <= row && row < 3);
      assert(0 <= col && col < 3);
      return m[col][row];
    }

    const float *data() const { return &m[0][0]; }
  };

  Mat3F operator*(const Mat3F& lhs, const Mat3F& rhs);

  Vec2F transform_point(const Mat3F& mat, Vec2F vec);
  Vec2F transform_vector(const Mat3F& mat, Vec2F vec);
  Mat3F inverse(const Mat3F& mat);

  Mat3F translation(Vec2F offset);

}

#endif // GAMMA_MATH_H
