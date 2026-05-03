#pragma once

#include <boost/thread.hpp>

#include <atomic>
#include <mutex>

namespace Fmi
{
class AsyncTask
{
 public:
  enum Status
  {
    none,
    active,
    ok,
    failed,
    interrupted
  };

  using nodify_finished_t = void(const std::string&, Status);

  static bool silent;

 public:
  /**
   *   @brief Constructor: creates AsyncTask object
   *
   *   @param name Name of task for logging purposes. Make is also to set thread name
   *               on platforms that support it. For this reason it is remmended to keep
   *               name length short (max 15 characters on Linux) to avoid truncation.
   *   @param task function to perform in the task
   *   @param notify function to call when requested function has ended (any reason - success,
   * interrupted, exception thrown)
   *
   *   Notification callback is called from task thread, so locking may be required
   */
  AsyncTask(const std::string& name,
            std::function<void()> task,
            std::function<void()> notify = std::function<void()>());

  virtual ~AsyncTask();

  void wait();

  /**
   *   @brief Stops the asynchronous task at interruption point
   */
  void cancel();

  bool wait_for(double sec);

  Status get_status() const;

  bool ended() const { return done; }

  const std::string& get_name() const { return name; }

  static void interruption_point();

 private:
  void run(std::function<void()> task);
  void handle_result(Status stat, std::exception_ptr exc = nullptr);
  std::exception_ptr get_exception() const;
  static void log_event_time(const AsyncTask* task, const std::string& desc);

  const std::string name;
  mutable std::mutex m1;
  std::atomic<Status> status;
  std::atomic<bool> done;
  const std::function<void()> notify;
  std::exception_ptr ex;
  boost::thread task_thread;

  AsyncTask(const AsyncTask&) = delete;
  AsyncTask(AsyncTask&&) = delete;
  AsyncTask& operator = (const AsyncTask&) = delete;
  AsyncTask& operator = (AsyncTask&&) = delete;

public:
  static bool log_time;
};
}  // namespace Fmi
