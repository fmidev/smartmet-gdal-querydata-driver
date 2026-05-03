#include "AsyncTask.h"

#include "DateTime.h"
#include "DebugTools.h"
#include "TypeName.h"
#include <chrono>
#include <iostream>
#include <fmt/format.h>
#include <boost/chrono.hpp>



bool Fmi::AsyncTask::silent = false;
bool Fmi::AsyncTask::log_time = false;

#define LOG_TIME(desc) if (Fmi::AsyncTask::log_time) { Fmi::AsyncTask::log_event_time(this, desc); }

Fmi::AsyncTask::AsyncTask(const std::string& name,
                          std::function<void()> task,
                          std::function<void()> notify)

    : name(name),
      status(none),
      done(false),
      notify(notify),
      ex(nullptr),
      task_thread([this, task]() { run(task); })
{
    LOG_TIME("created");
}

Fmi::AsyncTask::~AsyncTask()
{
  if (task_thread.joinable())
  {
    LOG_TIME("destructor entered");
    try
    {
      cancel();
      wait();
    }
    catch (const std::exception& e)
    {
      if (!silent)
      {
        std::cerr << '[' << METHOD_NAME << "] WARNING: Ignoring exception of type '"
                  << Fmi::current_exception_type() << "' from async task '" << name
                  << "': " << e.what() << std::endl;
      }
    }
    catch (...)
    {
      if (!silent)
      {
        std::cerr << '[' << METHOD_NAME << "] WARNING: Ignoring exception of type '"
                  << Fmi::current_exception_type() << "' from async task '" << name << std::endl;
      }
    }
    LOG_TIME("destructor done");
  }
}

void Fmi::AsyncTask::wait()
{
  if (task_thread.joinable())
  {
    LOG_TIME("join requested");
    task_thread.join();
    LOG_TIME("joined");
    std::exception_ptr exc = get_exception();
    if (exc)
    {
      std::rethrow_exception(exc);
    }
  }
}

bool Fmi::AsyncTask::wait_for(double sec)
{
  if (task_thread.joinable())
  {
    auto mks = int64_t(std::ceil(1000000.0 * sec));
    LOG_TIME(fmt::format("try_join for %.3 seconds requested", sec));
    bool is_done = task_thread.try_join_for(boost::chrono::microseconds(mks));
    LOG_TIME("joined");
    if (is_done)
    {
      std::exception_ptr exc = get_exception();
      if (exc)
      {
        std::rethrow_exception(exc);
      }
    }
    return is_done;
  }
  else
  {
    return false;
  }
}

void Fmi::AsyncTask::cancel()
{
  std::unique_lock<std::mutex> lock(m1);
  if (!done)
  {
    LOG_TIME("cancel requested");
    task_thread.interrupt();
  }
}

Fmi::AsyncTask::Status Fmi::AsyncTask::get_status() const
{
  return status;
}

void Fmi::AsyncTask::interruption_point()
{
  boost::this_thread::interruption_point();
}

void Fmi::AsyncTask::run(std::function<void()> task)
{
  try
  {
    status = active;
#ifdef BOOST_THREAD_PLATFORM_PTHREAD
    pthread_setname_np(pthread_self(), name.substr(0, 15).c_str());
#endif
    LOG_TIME("started");
    task();
    LOG_TIME("ended");
    handle_result(ok, nullptr);
  }
  catch (boost::thread_interrupted&)
  {
    LOG_TIME("interrupted");
    handle_result(interrupted, nullptr);
  }
  catch (...)
  {
    LOG_TIME(fmt::format("got interrupt {}", Fmi::current_exception_type()));
    handle_result(failed, std::current_exception());
  }
}

void Fmi::AsyncTask::handle_result(Status stat, std::exception_ptr exc)
{
  std::unique_lock<std::mutex> lock(m1);
  this->done = true;
  this->status = stat;
  this->ex = exc;
  lock.unlock();
  if (notify)
  {
    notify();
  }
}

std::exception_ptr Fmi::AsyncTask::get_exception() const
{
  std::unique_lock<std::mutex> lock(m1);
  return ex;
}

void Fmi::AsyncTask::log_event_time(const AsyncTask* task, const std::string& desc)
{
  std::cout << Fmi::MicrosecClock::local_time() << " [Fmi::AsyncTask]: ("
            << reinterpret_cast<const void*>(task) << ") '" << task->get_name()
            << "': " << desc
            << std::endl;
}

