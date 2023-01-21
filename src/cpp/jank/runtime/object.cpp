#include <fmt/compile.h>

#include <jank/runtime/obj/number.hpp>
#include <jank/runtime/obj/string.hpp>
#include <jank/runtime/obj/list.hpp>

namespace jank::runtime
{
  /* TODO: Replace these with non-static values. */
  // NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
  object_ptr JANK_NIL{ jank::make_box<obj::nil>() };

  namespace obj
  {
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
    object_ptr JANK_TRUE{ jank::make_box<boolean>(true) };
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
    object_ptr JANK_FALSE{ jank::make_box<boolean>(false) };
  }

  native_bool object::equal(object const &rhs) const
  { return to_hash() == rhs.to_hash(); }

  native_bool object::equal(object_ptr rhs) const
  { return rhs && equal(*rhs); }

  void object::to_string(fmt::memory_buffer &buffer) const
  { fmt::format_to(std::back_inserter(buffer), FMT_COMPILE("{}"), to_string()); }

  bool object::operator <(object const &rhs) const
  { return to_hash() < rhs.to_hash(); }

  native_bool obj::nil::equal(object const &o) const
  {
    auto const *n(o.as_nil());
    return n != nullptr;
  }
  native_string obj::nil::to_string() const
  /* TODO: Optimize. */
  { return "nil"; }
  native_integer obj::nil::to_hash() const
  { return 0; }
  obj::nil const* obj::nil::as_nil() const
  { return this; }

  object_ptr make_box(std::nullptr_t const &)
  { return JANK_NIL; }
  object_ptr make_box(native_bool const b)
  { return b ? obj::JANK_TRUE : obj::JANK_FALSE; }
  object_ptr make_box(int const i)
  { return jank::make_box<obj::integer>(native_integer{ i }); }
  object_ptr make_box(native_integer const i)
  { return jank::make_box<obj::integer>(i); }
  object_ptr make_box(native_real const r)
  { return jank::make_box<obj::real>(r); }
  object_ptr make_box(native_string_view const &s)
  { return jank::make_box<obj::string>(s); }
  object_ptr make_box(detail::persistent_list const &l)
  { return jank::make_box<obj::list>(l); }
}
