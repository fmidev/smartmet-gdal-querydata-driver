#include "StaticCleanup.h"

namespace Fmi
{
  std::list<std::function<void()>> StaticCleanup::m_cleanup_functions;

  std::atomic<unsigned> StaticCleanup::AtExit::m_instance_count = 0;

  StaticCleanup::StaticCleanup(std::function<void()> cleanup_function)
  {
    m_cleanup_functions.push_front(cleanup_function);
  }

  StaticCleanup::~StaticCleanup() = default;

  StaticCleanup::AtExit::AtExit()
  {
    ++m_instance_count;
  }

  StaticCleanup::AtExit::~AtExit()
  {
    if (--m_instance_count == 0)
    {
       for (const auto& cleanup_function : m_cleanup_functions)
       {
            cleanup_function();
       }
    }
  }
}
