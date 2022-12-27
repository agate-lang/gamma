#ifndef GAMMA_TAG_H
#define GAMMA_TAG_H

#include <cstdint>
#include <string>

#include "agate.h"

namespace gma {

  class Tag {
  public:
    constexpr Tag()
    : m_state(UINT64_C(0xcbf29ce484222325))
    {
    }

    constexpr void hash(const char *data, std::size_t size) {
      for (std::size_t i = 0; i < size; ++i) {
        m_state ^= static_cast<unsigned char>(data[i]);
        m_state *= UINT64_C(0x100000001b3);
      }
    }

    constexpr uint64_t result() const {
      return m_state;
    }

  private:
    uint64_t m_state;
  };

  constexpr uint64_t compute_tag(const char *unit_name, const char *class_name) {
    constexpr char separator = ':';

    Tag tag;
    tag.hash(unit_name, std::char_traits<char>::length(unit_name));
    tag.hash(&separator, 1);
    tag.hash(class_name, std::char_traits<char>::length(class_name));
    return tag.result();
  }

  template<typename Class>
  inline
  uint64_t generic_tag(AgateVM *vm, const char *unit_name, const char *class_name) {
    return Class::tag;
  }

}

#endif // GAMMA_TAG_H
