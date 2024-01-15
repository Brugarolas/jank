#pragma once

#include <functional>

/* TODO: Move file to detail dir */
namespace jank::runtime::detail
{
  /* Very much borrowed from boost. */
  template <typename T>
  native_integer hash_combine(size_t const seed, T const &t)
  {
    static std::hash<T> hasher{};
    /* TODO: Add an unsigned type to jank; the cast to signed here is going to
     * cause issues with overflows. */
    return static_cast<native_integer>(seed ^ hasher(t) + 0x9e3779b9 + (seed << 6) + (seed >> 2));
  }
}
