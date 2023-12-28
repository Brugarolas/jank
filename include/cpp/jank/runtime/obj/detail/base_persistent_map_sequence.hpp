#pragma once

#include <jank/runtime/object.hpp>

namespace jank::runtime::obj::detail
{
  template <object_type OT, typename It>
  struct base_persistent_map_sequence : gc
  {
    static constexpr bool pointer_free{ false };

    using parent_type = static_object<OT>;
    using iterator_type = It;

    base_persistent_map_sequence() = default;
    base_persistent_map_sequence(base_persistent_map_sequence &&) = default;
    base_persistent_map_sequence(base_persistent_map_sequence const &) = default;
    base_persistent_map_sequence(object_ptr c, iterator_type const &b, iterator_type const &e)
      : coll{ c }, begin{ b }, end{ e }
    { assert(begin != end); }

    /* behavior::objectable */
    native_bool equal(object const &o) const
    {
      return visit_object
      (
        [this](auto const typed_o)
        {
          using T = typename decltype(typed_o)::value_type;

          if constexpr(!behavior::seqable<T>)
          { return false; }
          else
          {
            auto seq(typed_o->fresh_seq());
            for(auto it(fresh_seq()); it != nullptr; seq = seq->next_in_place(), seq = seq->next_in_place())
            {
              if(seq == nullptr || !runtime::detail::equal(it, seq->first()))
              { return false; }
            }
            return true;
          }
        },
        &o
      );
    }

    void to_string_impl(fmt::memory_buffer &buff) const
    {
      auto inserter(std::back_inserter(buff));
      fmt::format_to(inserter, "(");
      for(auto i(begin); i != end; ++i)
      {
        fmt::format_to(inserter, "[");
        runtime::detail::to_string((*i).first, buff);
        fmt::format_to(inserter, " ");
        runtime::detail::to_string((*i).second, buff);
        fmt::format_to(inserter, "]");
        auto n(i);
        if(++n != end)
        { fmt::format_to(inserter, " "); }
      }
      fmt::format_to(inserter, ")");
    }

    void to_string(fmt::memory_buffer &buff) const
    { return to_string_impl(buff); }

    native_persistent_string to_string() const
    {
      fmt::memory_buffer buff;
      to_string_impl(buff);
      return native_persistent_string{ buff.data(), buff.size() };
    }

    native_integer to_hash() const
    /* TODO: Hash from contents. */
    { return reinterpret_cast<native_integer>(this); }

    /* behavior::countable */
    size_t count() const
    { return std::distance(begin, end); }

    /* behavior::seqable */
    native_box<parent_type> seq()
    { return static_cast<parent_type*>(this); }

    native_box<parent_type> fresh_seq() const
    { return jank::make_box<parent_type>(coll, begin, end); }

    /* behavior::sequenceable */
    object_ptr first() const
    {
      auto const pair(*begin);
      return jank::make_box<obj::vector>(runtime::detail::native_persistent_vector{ pair.first, pair.second });
    }

    native_box<parent_type> next() const
    {
      auto n(begin);
      ++n;

      if(n == end)
      { return nullptr; }

      return jank::make_box<parent_type>(coll, n, end);
    }

    native_box<parent_type> next_in_place()
    {
      ++begin;

      if(begin == end)
      { return nullptr; }

      return static_cast<parent_type*>(this);
    }
    object_ptr next_in_place_first()
    {
      ++begin;

      if(begin == end)
      { return nullptr; }

      auto const pair(*begin);
      return jank::make_box<obj::vector>(runtime::detail::native_persistent_vector{ pair.first, pair.second });
    }

    obj::cons_ptr cons(object_ptr const head)
    { return make_box<obj::cons>(head, static_cast<parent_type*>(this)); }

    object base{ OT };
    object_ptr coll{};
    iterator_type begin{}, end{};
  };
}
