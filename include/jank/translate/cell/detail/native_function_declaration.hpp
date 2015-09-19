#pragma once

#include <jank/translate/function/argument/definition.hpp>

namespace jank
{
  namespace interpret
  {
    namespace environment
    { struct scope; }
  }

  namespace translate
  {
    namespace environment
    { struct scope; }

    namespace cell
    {
      namespace detail
      {
        template <typename C>
        struct native_function_declaration
        {
          std::string name;
          function::argument::type_list<C> arguments;
          type_reference<C> return_type;
          std::shared_ptr<environment::scope> scope;
        };

        template <typename C>
        bool operator <
        (
          native_function_declaration<C> const &lhs,
          native_function_declaration<C> const &rhs
        )
        {
          if(lhs.name < rhs.name)
          { return true; }
          else if(rhs.name < lhs.name)
          { return false; }

          /* TODO: compare definitions directly. */
          return std::lexicographical_compare
          (
            lhs.arguments.begin(), lhs.arguments.end(),
            rhs.arguments.begin(), rhs.arguments.end(),
            [](auto const &l, auto const &r)
            { return l.type.definition.name < r.type.definition.name; }
          );
        }
      }
    }
  }
}
