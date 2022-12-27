#ifndef GAMMA_META_H
#define GAMMA_META_H

namespace gma {

  template<typename Type>
  struct TypeName {
  };

  template<typename Type>
  inline
  constexpr const char *type_name = TypeName<Type>::name;

  template<>
  struct TypeName<float> {
    static constexpr const char *name = "Float";
  };

  template<>
  struct TypeName<int> {
    static constexpr const char *name = "Int";
  };

}

#endif // GAMMA_META_H
