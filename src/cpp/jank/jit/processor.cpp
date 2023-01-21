#include <cling/Interpreter/Value.h>

#include <jank/util/process_location.hpp>
#include <jank/util/make_array.hpp>
#include <jank/jit/processor.hpp>

namespace jank::jit
{
  option<boost::filesystem::path> find_pch()
  {
    auto const jank_path(jank::util::process_location().unwrap().parent_path());

    auto dev_path(jank_path / "CMakeFiles/jank_lib.dir/cmake_pch.hxx.pch");
    if(boost::filesystem::exists(dev_path))
    { return std::move(dev_path); }

    auto installed_path(jank_path / "../include/cmake_pch.hxx.pch");
    if(boost::filesystem::exists(installed_path))
    { return std::move(installed_path); }

    return none;
  }

  processor::processor()
  {
    auto const pch_path(find_pch());
    if(pch_path.is_none())
    /* TODO: Better error handling. */
    { throw std::runtime_error{ "unable to find PCH path for JIT" }; }
    auto const &pch_path_str(pch_path.unwrap().string());

    auto const args
    (
      jank::util::make_array
      (
        "clang++", "-std=c++17",
        "-DHAVE_CXX14=1", "-DIMMER_HAS_LIBGC=1",
        "-include-pch", pch_path_str.c_str()
      )
    );
    interpreter = std::make_unique<cling::Interpreter>(args.size(), args.data(), LLVMDIR);
    interpreter->setDefaultOptLevel(1);
  }

  result<option<runtime::object_ptr>, native_string> processor::eval
  (runtime::context &, codegen::processor &cg_prc) const
  {
    /* TODO: Improve Cling to accept string_views instead. */
    interpreter->declare(static_cast<std::string>(cg_prc.declaration_str()));

    auto const expr(cg_prc.expression_str(false));
    if(expr.empty())
    { return ok(none); }

    cling::Value v;
    auto const result(interpreter->evaluate(static_cast<std::string>(expr), v));
    if(result != cling::Interpreter::CompilationResult::kSuccess)
    { return err("compilation error"); }

    auto * const ret_val(v.simplisticCastAs<runtime::object_ptr>());
    return ok(ret_val);
  }

  void processor::eval_string(native_string const &s) const
  { interpreter->process(static_cast<std::string>(s)); }
}
