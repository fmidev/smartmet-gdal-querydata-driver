#include <atomic>
#include <functional>
#include <list>

namespace Fmi
{
  /**
   * @brief StaticCleanup class provides a way to register cleanup functions
   *
   * StaticCleanup class provides a way to register cleanup functions that are
   * called when the last instance of AtExit class is destroyed.
   *
   * The cleanup functions are called in the reverse order they were registered.
   *
   * It is intended for use of global/static objects that need to be cleaned up
   * when the program exits.
   */
  class StaticCleanup final
  {
    public:
      StaticCleanup(std::function<void()> cleanup_function);
      ~StaticCleanup();

      class AtExit final
      {
        public:
          AtExit();
          ~AtExit();

        private:
          static std::atomic<unsigned> m_instance_count;
      };

    private:
       static std::list<std::function<void()>> m_cleanup_functions;
  };
}
