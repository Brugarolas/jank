#pragma once

#include <jank/runtime/object.hpp>

namespace jank::runtime
{
  template <>
  struct static_object<object_type::nil> : gc
  {
    static constexpr bool pointer_free{ true };

    static native_box<static_object> nil_const();

    static_object() = default;
    static_object(object &&base);

    /* behavior::objectable */
    native_bool equal(object const &) const;
    native_persistent_string const& to_string() const;
    void to_string(fmt::memory_buffer &buff) const;
    native_integer to_hash() const;

    /* behavior::associatively_readable */
    object_ptr get(object_ptr const key);
    object_ptr get(object_ptr const key, object_ptr const fallback);
    object_ptr get_entry(object_ptr key);
    native_bool contains(object_ptr key) const;

    /* behavior::seqable */
    native_box<static_object> seq();
    native_box<static_object> fresh_seq() const;

    /* behavior::sequenceable */
    native_box<static_object> first() const;
    native_box<static_object> next() const;
    native_box<static_object> next_in_place();
    native_box<static_object> next_in_place_first();

    object base{ object_type::nil };
  };

  namespace obj
  {
    using nil = static_object<object_type::nil>;
    using nil_ptr = native_box<nil>;
  }
}