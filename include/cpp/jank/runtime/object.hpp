#pragma once

#include <experimental/iterator>
#include <type_traits>
#include <any>
#include <functional>
#include <memory>
#include <map>
#include <string_view>

#include <fmt/format.h>

#include <immer/vector.hpp>
#include <immer/vector_transient.hpp>
#include <immer/map.hpp>
#include <immer/map_transient.hpp>
#include <immer/set.hpp>
#include <immer/set_transient.hpp>
#include <immer/memory_policy.hpp>

#include <jank/runtime/behavior/callable.hpp>
#include <jank/runtime/behavior/metadatable.hpp>
#include <jank/runtime/detail/type.hpp>
#include <jank/runtime/detail/list_type.hpp>
#include <jank/runtime/detail/string_type.hpp>
#include <jank/runtime/detail/map_type.hpp>

namespace jank::runtime
{
  namespace obj
  {
    struct nil;
    struct boolean;
    struct integer;
    struct real;
    struct number;
    struct string;
    struct symbol;
    struct keyword;
    struct list;
    struct vector;
    struct map;
    struct set;
    struct function;
  }
  namespace behavior
  {
    struct callable;
    struct seqable;
    struct countable;
  }

  struct var;
  struct ns;

  //using object_ptr = detail::box_type<struct object>;
  using object_ptr = struct object*;
  struct object : virtual gc
  {
    virtual detail::boolean_type equal(object const &) const;
    detail::boolean_type equal(object_ptr) const;
    virtual detail::string_type to_string() const = 0;
    virtual void to_string(fmt::memory_buffer &buffer) const;
    virtual detail::integer_type to_hash() const = 0;

    bool operator <(object const &) const;

    /* TODO: Benchmark what it's like to store a pointer of each type instead; no more dynamic dispactch. */
    /* TODO: Benchmark the impact of using option here. */
    virtual var const* as_var() const
    { return nullptr; }
    virtual ns const* as_ns() const
    { return nullptr; }
    virtual obj::nil const* as_nil() const
    { return nullptr; }
    virtual obj::boolean const* as_boolean() const
    { return nullptr; }
    virtual obj::integer const* as_integer() const
    { return nullptr; }
    virtual obj::real const* as_real() const
    { return nullptr; }
    virtual obj::number const* as_number() const
    { return nullptr; }
    virtual obj::string const* as_string() const
    { return nullptr; }
    virtual obj::symbol const* as_symbol() const
    { return nullptr; }
    virtual obj::keyword const* as_keyword() const
    { return nullptr; }
    virtual obj::list const* as_list() const
    { return nullptr; }
    virtual obj::vector const* as_vector() const
    { return nullptr; }
    virtual obj::map const* as_map() const
    { return nullptr; }
    virtual obj::set const* as_set() const
    { return nullptr; }
    virtual behavior::seqable const* as_seqable() const
    { return nullptr; }
    virtual obj::function const* as_function() const
    { return nullptr; }
    virtual behavior::callable const* as_callable() const
    { return nullptr; }
    virtual behavior::metadatable const* as_metadatable() const
    { return nullptr; }
    virtual behavior::countable const* as_countable() const
    { return nullptr; }
  };

  inline std::ostream& operator<<(std::ostream &os, object const &o)
  /* TODO: Optimize this by using virtual dispatch to write into the stream, rather than allocating a string. */
  { return os << o.to_string(); }

  namespace detail
  {
    struct object_ptr_equal
    {
      static bool equal(object_ptr l, object_ptr r)
      { return l == r || l->equal(*r); }

      inline bool operator()(object_ptr l, object_ptr r) const
      { return equal(l, r); }
    };
    struct object_ptr_less
    {
      inline bool operator()(object_ptr l, object_ptr r) const
      {
        auto const l_hash(l->to_hash());
        auto const r_hash(r->to_hash());
        return l_hash < r_hash;
      }
    };

    using list_type = list_type_impl<object_ptr>;
    using vector_type = immer::vector<object_ptr, detail::memory_policy>;
    using vector_transient_type = vector_type::transient_type;
    using set_type = immer::set<object_ptr, std::hash<object_ptr>, std::equal_to<>, detail::memory_policy>;
    using set_transient_type = set_type::transient_type;
    //using map_type = immer::map<object_ptr, object_ptr, std::hash<object_ptr>, object_ptr_equal, detail::memory_policy>;
    //using map_type = std::map<object_ptr, object_ptr, object_ptr_less>;
    using map_type = map_type_impl<object_ptr, object_ptr>;
    //using map_transient_type = map_type::transient_type;
  }

  namespace obj
  {
    struct nil : object
    {
      nil() = default;
      nil(nil &&) noexcept = default;
      nil(nil const &) = default;
      ~nil() = default;

      runtime::detail::boolean_type equal(object const &) const override;
      runtime::detail::string_type to_string() const override;
      runtime::detail::integer_type to_hash() const override;

      nil const* as_nil() const override;
    };

    extern object_ptr JANK_TRUE; // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)
    extern object_ptr JANK_FALSE; // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)
  }
  extern object_ptr JANK_NIL; // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)


  object_ptr make_box(std::nullptr_t const &);
  object_ptr make_box(bool const);
  object_ptr make_box(int const);
  object_ptr make_box(detail::integer_type const);
  object_ptr make_box(detail::real_type const);
  object_ptr make_box(std::string_view const &);
  object_ptr make_box(detail::list_type const &);

  template <typename T>
  inline T* make_box(T* const &o)
  { return o; }
  template <typename C>
  C* make_box()
  { return new (GC) C{}; }
  template <typename C, typename... Args>
  C* make_box(Args &&... args)
  { return new (GC) C{ std::forward<Args>(args)... }; }
}

namespace std
{
  template <>
  struct hash<jank::runtime::object>
  {
    size_t operator()(jank::runtime::object const &o) const noexcept
    { return static_cast<size_t>(o.to_hash()); }
  };

  template <>
  struct hash<jank::runtime::object_ptr>
  {
    size_t operator()(jank::runtime::object_ptr o) const noexcept
    {
      static auto hasher(std::hash<jank::runtime::object>{});
      return hasher(*o);
    }
  };

  template <>
  struct equal_to<jank::runtime::object_ptr>
  {
    bool operator()(jank::runtime::object_ptr lhs, jank::runtime::object_ptr rhs) const noexcept
    {
      if(!lhs)
      { return !rhs; }
      else if(!rhs)
      { return !lhs; }
      return lhs->equal(*rhs);
    }
  };
}
