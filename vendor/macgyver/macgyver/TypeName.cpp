#include "TypeName.h"
#include <boost/core/demangle.hpp>
#include <cstdlib>

std::string Fmi::demangle_cpp_type_name(const std::string& src)
{
  return boost::core::demangle(src.c_str());
}

std::string Fmi::current_exception_type()
{
  return demangle_cpp_type_name(abi::__cxa_current_exception_type()->name());
}
