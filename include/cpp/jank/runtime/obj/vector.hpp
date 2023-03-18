#pragma once

#include <jank/runtime/behavior/seqable.hpp>
#include <jank/runtime/behavior/countable.hpp>
#include <jank/runtime/behavior/consable.hpp>
#include <jank/runtime/behavior/metadatable.hpp>
#include <jank/runtime/behavior/associatively_readable.hpp>

namespace jank::runtime::obj
{
  struct vector
    :
      virtual object,
      behavior::seqable, behavior::countable, behavior::consable,
      behavior::metadatable,
      behavior::associatively_readable
  {
    vector() = default;
    vector(vector &&) = default;
    vector(vector const &) = default;
    vector(runtime::detail::peristent_vector &&d);
    vector(runtime::detail::peristent_vector const &d);
    template <typename... Args>
    vector(Args &&...args)
      : data{ std::forward<Args>(args)... }
    { }
    ~vector() = default;

    /* TODO: Remove; dupe of ctor. */
    static native_box<vector> create(runtime::detail::peristent_vector const &);
    static native_box<vector> create(behavior::sequence_ptr const &s);

    native_bool equal(object const &) const override;
    native_string to_string() const override;
    void to_string(fmt::memory_buffer &buff) const override;
    native_integer to_hash() const override;

    vector const* as_vector() const override;
    behavior::seqable const* as_seqable() const override;

    behavior::sequence_ptr seq() const override;
    size_t count() const override;

    behavior::consable const* as_consable() const override;
    native_box<behavior::consable> cons(object_ptr head) const override;

    object_ptr with_meta(object_ptr m) const override;
    behavior::metadatable const* as_metadatable() const override;

    behavior::associatively_readable const* as_associatively_readable() const override;
    object_ptr get(object_ptr key) const override;
    object_ptr get(object_ptr key, object_ptr fallback) const override;

    runtime::detail::peristent_vector data;
  };
  using vector_ptr = native_box<vector>;
}
