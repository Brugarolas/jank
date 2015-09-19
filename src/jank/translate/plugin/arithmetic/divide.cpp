#include <jank/translate/plugin/arithmetic/divide.hpp>
#include <jank/translate/plugin/arithmetic/detail/make_operator.hpp>
#include <jank/interpret/environment/resolve_value.hpp>
#include <jank/interpret/expect/type.hpp>

namespace jank
{
  namespace translate
  {
    namespace plugin
    {
      namespace arithmetic
      {
        void divide(std::shared_ptr<environment::scope> const &scope)
        {
          detail::make_operator
          (scope, "/", environment::builtin::type::integer(*scope));
          detail::make_operator
          (scope, "/", environment::builtin::type::real(*scope));
        }
      }
    }
  }
}
