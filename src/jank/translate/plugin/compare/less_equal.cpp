#include <jank/translate/plugin/compare/less_equal.hpp>
#include <jank/translate/plugin/compare/detail/make_operator.hpp>
#include <jank/interpret/environment/resolve_value.hpp>
#include <jank/interpret/expect/type.hpp>

namespace jank
{
  namespace translate
  {
    namespace plugin
    {
      namespace compare
      {
        void less_equal(std::shared_ptr<environment::scope> const &scope)
        {
          detail::make_operator
          (scope, "<=", environment::builtin::type::integer(*scope));
          detail::make_operator
          (scope, "<=", environment::builtin::type::real(*scope));
          detail::make_operator
          (scope, "<=", environment::builtin::type::string(*scope));
        }
      }
    }
  }
}
