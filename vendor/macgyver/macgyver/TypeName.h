/**
 *   @file TypeName.h
 *   @brief Convenience functions and macros for getting actual C++ type name in human readable
 * format
 *
 *   Some possible uses are:
 *   - getting actual name of the C++ exception thrown when
 *       - base class exception (for example std::exception) is catched
 *       - unspecified exception is catched (catch(...))
 *   - getting actual derived class name from base class member function to be used in output
 * messages
 *
 *   @note It would perhaps be useful to move this stuff to @b mcgyver later
 */

#pragma once

#include <string>
#include <typeinfo>

namespace Fmi
{
/**
 *  @brief Returns human readable actual type name from output of std::typeinfo::name()
 */
std::string demangle_cpp_type_name(const std::string& src);

/**
 *  @brief Returns human readable actual type name of the object pointer to which is given as an
 * argument
 *
 *  Can be used to find the name of object of derived class when base class pointer is available
 */
template <typename Type>
std::string get_type_name(const Type* x)
{
  return demangle_cpp_type_name(typeid(*x).name());
}

/**
 *  @brief Returns type name of an thrown object when called inside the catch block
 */
std::string current_exception_type();
}  // namespace Fmi

// Works with Visual Studio too
#define METHOD_NAME (Fmi::get_type_name(this) + "::" + __FUNCTION__)
