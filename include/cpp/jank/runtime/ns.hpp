#pragma once

#include <functional>
#include <unordered_map>
#include <mutex>

#include <libguarded/shared_guarded.hpp>

#include <jank/runtime/obj/symbol.hpp>
#include <jank/runtime/var.hpp>

namespace jank::runtime
{
  struct var;
  struct context;

  struct ns : object
  {
    using ns_ptr = native_box<ns>;

    static constexpr bool pointer_free{ false };

    ns(obj::symbol_ptr const &name, context const &c)
      : name{ name }, rt_ctx{ c }
    { }
    virtual ~ns() = default;

    static ns_ptr create(obj::symbol_ptr const &n, context const &c);

    native_bool equal(object const &) const final;
    native_string to_string() const final;
    void to_string(fmt::memory_buffer &buff) const final;
    native_integer to_hash() const final;

    ns const* as_ns() const final;

    bool operator ==(ns const &rhs) const;

    ns_ptr clone() const;

    obj::symbol_ptr name{};
    libguarded::shared_guarded<native_unordered_map<obj::symbol_ptr, var_ptr>> vars;
    context const &rt_ctx;
  };
  using ns_ptr = native_box<ns>;
}

namespace std
{
  template <>
  struct hash<jank::runtime::var>
  {
    size_t operator()(jank::runtime::var const &o) const noexcept
    { return static_cast<size_t>(o.n->name->to_hash()); }
  };
}
