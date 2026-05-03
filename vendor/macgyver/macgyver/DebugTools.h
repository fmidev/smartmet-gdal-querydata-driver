#pragma once

#include "TypeName.h"
#include "Exception.h"
#include <exception>
#include <iostream>
#include <string>

namespace Fmi
{
/**
 *   Class that performing logging of its object life time
 *
 *   To use create local object of this class in scope which
 *   execution time must be measured, like:
 *
 *   @code
 *   void foo()
 *   {
 *      Fmi::ScopedTimer timer(__FUNCTION__);
 *      // do something ...
 *      ...
 *   }
 *   @endcode
 */
class ScopedTimer
{
 public:
  ScopedTimer(const std::string& theName);
  virtual ~ScopedTimer();

 private:
  const std::string name;
  std::string time_str;
  double start;
};

class Redirecter final
{
public:
    Redirecter(std::ostream& dest, std::ostream& src);
    ~Redirecter();

    Redirecter(const Redirecter&) = delete;
    Redirecter& operator = (const Redirecter&) = delete;

private:
    std::ostream& src;
    std::streambuf* sbuf;
};

/**
 *  @brief Returns tracer PID under Linux (or always 0 for Windows)
 */
int tracerPid();

/**
 *   @brief Debugging macro for showing exceptions thrown by a function
 *
 *   Possible use:
 *   boost test framwork do not report exceptions type and related info in case of use
 *   of macros line BOOST_CHECK_NO_THROW(). On can use this macro to workaround this
 *   problem:
 *   @code
 *   BOOST_CHECK_NO_THROW(foo = SHOW_EXCEPTIONS(bar());
 *   @endcode
 */
#define SHOW_EXCEPTIONS(x)                                                                        \
  [&]()                                                                                           \
  {                                                                                               \
    try                                                                                           \
    {                                                                                             \
      return x;                                                                                   \
    }                                                                                             \
    catch (...)                                                                                   \
    {                                                                                             \
      Fmi::Exception::ForceStackTrace forceStackTrace;                                            \
      const auto e = Fmi::Exception::Trace(BCP,                                                   \
          "SHOW_EXCEPTIONS: Exception thrown by '"#x"'");                                         \
      std::cout << e << std::endl;                                                                \
      throw; /* Rethrow the original exception. Newly created exception object is for  */         \
      /* output only   */                                                                         \
    }                                                                                             \
  }()

}  // namespace Fmi
