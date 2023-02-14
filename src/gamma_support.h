#ifndef GAMMA_SUPPORT_H
#define GAMMA_SUPPORT_H

#include <filesystem>
#include <map>
#include <optional>
#include <tuple>
#include <type_traits>
#include <vector>

#include "agate.h"

#include "gamma_tag.h"

namespace gma {

  class Support {
  public:
    void add_include_path(std::filesystem::path path);
    const char *load_unit(const char *unit_name);

    void add_class_handler(const char *unit_name, const char *class_name, AgateForeignClassHandler handler);

    void add_method(const char *unit_name, const char *class_name, AgateForeignMethodKind kind, const char *signature, AgateForeignMethodFunc func);

    static AgateUnitHandler unit_handler(AgateVM *vm, const char *name);
    static AgateForeignClassHandler foreign_class_handler(AgateVM *vm, const char *unit_name, const char *class_name);
    static AgateForeignMethodFunc foreign_method_handler(AgateVM *vm, const char *unit_name, const char *class_name, AgateForeignMethodKind kind, const char *signature);

  private:
    const char *try_load_unit_file(const std::filesystem::path& filename);
    const char *try_load_unit(const std::filesystem::path& unit_name);
    static const char *unit_load(const char *name, void *user_data);

    AgateForeignClassHandler get_class_handler(const char *unit_name, const char *class_name);
    AgateForeignMethodFunc get_method(const char *unit_name, const char *class_name, AgateForeignMethodKind kind, const char *signature);


  private:
    std::vector<std::filesystem::path> m_include_paths;
    std::map<std::filesystem::path, std::string> m_units;
    std::map<std::tuple<std::string, std::string>, AgateForeignClassHandler> m_class_handlers;
    std::map<std::tuple<std::string, std::string, AgateForeignMethodKind, std::string>, AgateForeignMethodFunc> m_methods;
  };

  template<typename Class>
  ptrdiff_t generic_allocate([[maybe_unused]] AgateVM *vm, [[maybe_unused]] const char *unit_name, [[maybe_unused]] const char *class_name) {
    static_assert(std::is_trivial_v<typename Class::type>, "type should be trivial");
    return sizeof(typename Class::type);
  }

  template<typename Class>
  AgateForeignClassHandler generic_simple_handler() {
    return { generic_allocate<Class>, generic_tag<Class>, nullptr };
  }

  template<typename Class>
  AgateForeignClassHandler generic_handler(AgateForeignDestroyFunc destroy) {
    return { generic_allocate<Class>, generic_tag<Class>, destroy };
  }

}

#endif // GAMMA_SUPPORT_H
