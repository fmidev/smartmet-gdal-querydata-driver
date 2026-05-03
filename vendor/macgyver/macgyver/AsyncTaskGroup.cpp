#include "AsyncTaskGroup.h"
#include <algorithm>
#include <sstream>
#include "Exception.h"
#include "TypeName.h"

Fmi::AsyncTaskGroup::AsyncTaskGroup(std::size_t max_paralell_tasks)
    : counter(0),
      max_paralell_tasks(max_paralell_tasks),
      num_suceeded(0),
      num_failed(0),
      stop_requested(false),
      stop_on_error_(false)
{
}

Fmi::AsyncTaskGroup::~AsyncTaskGroup()
{
  stop();
  wait();
}

void Fmi::AsyncTaskGroup::add(const std::string& name, const std::function<void()>& task)
{
  while (get_num_active_tasks() >= max_paralell_tasks)
  {
    wait_some();
  }

  std::unique_lock<std::mutex> lock(m1);
  if (!stop_requested)
  {
    std::size_t task_id = ++counter;
    std::shared_ptr<AsyncTask> new_task(new AsyncTask(
        name, task,
        [this, task_id] ()
        {
            on_task_completed_callback(task_id);
        }));
    active_tasks[task_id] = new_task;
  }
}

void Fmi::AsyncTaskGroup::wait()
{
  std::exception_ptr first_exception;
  const auto wait_some_wrapper =
      [this, &first_exception] () -> bool
      {
          try {
              return wait_some();
          } catch (...) {
              if (not first_exception) {
                  first_exception = std::current_exception();
              }
              // FIXME: should we report other exceptions there if any
              return true;
          }
      };

  while (wait_some_wrapper())
  {
  }

  if (first_exception) {
      std::rethrow_exception(first_exception);
  }
}

void Fmi::AsyncTaskGroup::stop()
{
  std::unique_lock<std::mutex> lock(m1);
  if (not stop_requested.exchange(true))
  {
    for (const auto& item : active_tasks)
    {
      item.second->cancel();
    }
  }
}

std::size_t Fmi::AsyncTaskGroup::get_num_active_tasks() const
{
  std::unique_lock<std::mutex> lock(m1);
  return active_tasks.size();
}

std::vector<std::string> Fmi::AsyncTaskGroup::active_task_names() const
{
  std::vector<std::string> result;
  std::unique_lock<std::mutex> lock(m1);
  std::transform(active_tasks.begin(), active_tasks.end(), std::back_inserter(result),
      [](const decltype(active_tasks)::value_type& item) { return item.second->get_name(); });
  return result;
}

std::list<std::pair<std::string, std::exception_ptr> >
Fmi::AsyncTaskGroup::get_exception_info() const
{
  std::unique_lock<std::mutex> lock(m1);
  return exception_info;
}

std::list<std::pair<std::string, std::exception_ptr> >
Fmi::AsyncTaskGroup::get_and_clear_exception_info()
{
  std::list<std::pair<std::string, std::exception_ptr> > tmp;
  std::unique_lock<std::mutex> lock(m1);
  std::swap(exception_info, tmp);
  lock.unlock();
  return tmp;
}

void Fmi::AsyncTaskGroup::dump_and_clear_exception_info(std::ostream& output_stream)
{
  const auto exc_info = get_and_clear_exception_info();
  for (const auto& item : exc_info)
  {
    try
    {
      std::rethrow_exception(item.second);
    }
    catch (...)
    {
      std::ostringstream msg;
      msg << "Fmi::AsyncTaskGroup: task '" << item.first
          << "' terminated by exception of type '"
          << Fmi::current_exception_type() << '\'';
      const auto e  = Fmi::Exception::Trace(BCP, msg.str());
      output_stream << e << std::endl;
    }
  }
}

boost::signals2::connection Fmi::AsyncTaskGroup::on_task_ended(
    const std::function<void(const std::string&)>& callback)
{
  return signal_task_ended.connect(callback);
}

boost::signals2::connection Fmi::AsyncTaskGroup::on_task_error(
    const std::function<void(const std::string&)>& callback)
{
  return signal_task_failed.connect(callback);
}

bool Fmi::AsyncTaskGroup::stop_on_error(bool enable)
{
  return stop_on_error_.exchange(enable);
}

bool Fmi::AsyncTaskGroup::wait_some()
{
  std::unique_lock<std::mutex> lock(m1);
  cond.wait(lock,
            [this]()
            {
              return active_tasks.empty() || !completed_tasks.empty();
            });
  lock.unlock();

  return handle_finished();
}

bool Fmi::AsyncTaskGroup::handle_finished()
{
  std::unique_lock<std::mutex> lock2(m1);
  if (completed_tasks.empty())
    return false;

  bool some_failed = false;
  std::shared_ptr<AsyncTask> task = completed_tasks.front();
  completed_tasks.pop();
  lock2.unlock();
  try
  {
    task->wait();
    // Notify through signal only if done not interrupted
    if (task->get_status() == AsyncTask::ok)
    {
      signal_task_ended(task->get_name());
      num_suceeded++;
    }
  }
  catch (...)
  {
    some_failed = true;
    num_failed++;
    while (exception_info.size() >= MAX_EXCEPTIONS) {
      exception_info.pop_front();
    }
    exception_info.emplace_back(task->get_name(), std::current_exception());
    signal_task_failed(task->get_name());
  }

  if (stop_on_error_ && some_failed)
  {
    stop();
    throw Fmi::Exception(BCP,
                         "One ore more tasks failed with C++ exceptions."
                         " Stopping remaining tasks");
  }

  return true;
}

void Fmi::AsyncTaskGroup::on_task_completed_callback(std::size_t task_id)
{
  std::unique_lock<std::mutex> lock(m1);
  auto it = active_tasks.find(task_id);
  if (it == active_tasks.end())
  {
    // Should never happen
    throw Fmi::Exception(BCP,
                         " [INTERNAL ERROR] : task " + std::to_string(task_id) + " is not found");
  }

  completed_tasks.push(it->second);
  active_tasks.erase(it);
  lock.unlock();
  cond.notify_all();
}
