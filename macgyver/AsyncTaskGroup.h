#pragma once

#include "AsyncTask.h"
#include <boost/signals2.hpp>
#include <atomic>
#include <condition_variable>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <queue>
#include <vector>

namespace Fmi
{
class AsyncTaskGroup
{
 public:
  explicit AsyncTaskGroup(std::size_t max_paralell_tasks = 30);
  virtual ~AsyncTaskGroup();

  /**
   *   @brief Add a new task
   *
   *   May block if the number of maximal paralell tasks is achieved
   *   (wait for some tasks ton complete)
   */
  void add(const std::string& name, const std::function<void()>& task);

  /**
   *   @brief Wait for all added tasks to end
   *
   *   Any type of ending is accepted:
   *   - success      - callback provided in call to method on_task_ended is called
   *   - failed       - unhandled C++ exception in task. Callback provided in call to
   *                    method on_task_failed is called from exception handler
   *   - interrupted  - task ended due to call to method stop
   */
  void wait();

  /**
   *   @brief Handle completion of already finished tasks (if there is any) and do not
   *          for any more to finish
   *
   *   Used callbacks are same as for method wait (wait uses handle_finished internally)
   */
  bool handle_finished();

  /**
   *   @brief Requests all active tasks to stop and do not accept any more
   */
  void stop();

  /**
   *  @brief Total number of tasks added
   */
  std::size_t get_task_count() const { return counter; }

  /**
   *  @brief Get number of tasks ended successfully
   */
  std::size_t get_tasks_succeeded() const { return num_suceeded; }

  /**
   *  @brief Get list of C++ exceptions that has caused async tasks to fail
   *
   *  Only last MAX_EXCEPTIONS entries are preserved
   */
  std::list<std::pair<std::string, std::exception_ptr> > get_exception_info() const;

  /**
   *  @brief Get list of C++ exceptions that has caused async tasks to fail and clear it
   */
  std::list<std::pair<std::string, std::exception_ptr> > get_and_clear_exception_info();

  /**
   *  @brief Dump information C++ about exceptions that has caused async tasks to fail
   *         and clear exception info
   */
  void dump_and_clear_exception_info(std::ostream& output_stream);

  /**
   *  @brief Get number of failed tasks
   *
   *  Only those which has thrown an exception. Canceled tasks are not included
   */
  std::size_t get_num_failures() const { return num_failed; }

  std::size_t get_num_active_tasks() const;

  /**
   *  @brief Get names of active tasks
   */
  std::vector<std::string> active_task_names() const;

  /**
   *  @brief Specifies callback to be called when task has ended succesfully
   *
   *  Called from method wait()
   */
  boost::signals2::connection on_task_ended(
      const std::function<void(const std::string&)>& callback);

  /**
   *  @brief Specifies callback to be called when task has failed (threw an exception)
   *
   *  Called from method wait() while handling the exception.
   */
  boost::signals2::connection on_task_error(
      const std::function<void(const std::string&)>& callback);

  /**
   *  @brief Specifies whether all tasks should interrupted when one or more are noticed failing
   *         (exception thrown)
   */
  bool stop_on_error(bool enable);

 private:
  bool wait_some();
  void on_task_completed_callback(std::size_t task_id);

  std::atomic_size_t counter;
  std::size_t max_paralell_tasks;
  mutable std::mutex m1;
  std::condition_variable cond;
  std::map<std::size_t, std::shared_ptr<AsyncTask> > active_tasks;
  std::queue<std::shared_ptr<AsyncTask> > completed_tasks;
  std::atomic_size_t num_suceeded;
  std::atomic_size_t num_failed;
  boost::signals2::signal<void(const std::string&)> signal_task_ended;
  boost::signals2::signal<void(const std::string&)> signal_task_failed;
  std::atomic_bool stop_requested;
  std::atomic_bool stop_on_error_;

  /**
   *   @brief Information about exceptions that have caused termination of async tasks
   */
  std::list<std::pair<std::string, std::exception_ptr> > exception_info;

  static constexpr std::size_t MAX_EXCEPTIONS = 100;

  AsyncTaskGroup(const AsyncTaskGroup&) = delete;
  AsyncTaskGroup(AsyncTaskGroup&&) = delete;
  AsyncTaskGroup& operator=(const AsyncTaskGroup&) = delete;
  AsyncTaskGroup& operator=(AsyncTaskGroup&&) = delete;
};
}  // namespace Fmi
