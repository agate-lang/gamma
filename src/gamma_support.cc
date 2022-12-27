#include "gamma_support.h"

#include <cassert>
#include <fstream>
#include <sstream>

namespace gma {

  /*
   * unit handling
   */

  void Support::add_include_path(std::filesystem::path path) {
    assert(std::filesystem::is_directory(path));
    m_include_paths.push_back(std::move(path));
  }

  const char *Support::load_unit(const char *name) {
    std::filesystem::path unit_name(name);

    if (unit_name.extension() == ".agate") {
      return try_load_unit_file(unit_name);
    }

    return try_load_unit(unit_name);
  }

  const char *Support::try_load_unit_file(const std::filesystem::path& filename) {
    if (auto iterator = m_units.find(filename); iterator != m_units.end()) {
      return iterator->second.c_str();
    }

    std::ifstream file(filename, std::ios::in | std::ios::binary);

    if (file) {
      std::ostringstream content;
      content << file.rdbuf();

      auto [ iterator, inserted ] = m_units.emplace(filename, content.str());
      assert(inserted);
      return iterator->second.c_str();
    }

    return nullptr;
  }

  const char *Support::try_load_unit(const std::filesystem::path& unit_name) {
    std::filesystem::path unit_filename(unit_name);
    unit_filename.replace_extension("agate");

    for (auto & include_path : m_include_paths) {
      std::filesystem::path filename = include_path / unit_filename;

      auto maybe_content = try_load_unit_file(filename);

      if (maybe_content) {
        return maybe_content;
      }
    }

    return nullptr;
  }

  const char *Support::unit_load(const char *name, void *user_data) {
    Support *support = static_cast<Support *>(user_data);
    return support->try_load_unit(name);
  }

  /*
   * foreign class handling
   */

  void Support::add_class_handler(const char *unit_name, const char *class_name, AgateForeignClassHandler handler) {
    m_class_handlers[std::make_tuple(unit_name, class_name)] = handler;
  }

  AgateForeignClassHandler Support::get_class_handler(const char *unit_name, const char *class_name) {
    AgateForeignClassHandler null_handler = { nullptr, nullptr, nullptr };

    if (auto iterator = m_class_handlers.find(std::make_tuple(unit_name, class_name)); iterator != m_class_handlers.end()) {
      return iterator->second;
    }

    return null_handler;
  }

  /*
   * foreign method handling
   */

  void Support::add_method(const char *unit_name, const char *class_name, AgateForeignMethodKind kind, const char *signature, AgateForeignMethodFunc func) {
    m_methods[std::make_tuple(unit_name, class_name, kind, signature)] = func;
  }

  AgateForeignMethodFunc Support::get_method(const char *unit_name, const char *class_name, AgateForeignMethodKind kind, const char *signature) {
    if (auto iterator = m_methods.find(std::make_tuple(unit_name, class_name, kind, signature)); iterator != m_methods.end()) {
      return iterator->second;
    }

    return nullptr;
  }

  /*
   * configuration
   */

  AgateUnitHandler Support::unit_handler(AgateVM *vm, [[maybe_unused]] const char *name) {
    return { Support::unit_load, nullptr, agateGetUserData(vm) };
  }

  AgateForeignClassHandler Support::foreign_class_handler(AgateVM *vm, const char *unit_name, const char *class_name) {
    Support *support = static_cast<Support *>(agateGetUserData(vm));
    return support->get_class_handler(unit_name, class_name);
  }

  AgateForeignMethodFunc Support::foreign_method_handler(AgateVM *vm, const char *unit_name, const char *class_name, AgateForeignMethodKind kind, const char *signature) {
    Support *support = static_cast<Support *>(agateGetUserData(vm));
    return support->get_method(unit_name, class_name, kind, signature);
  }

}
