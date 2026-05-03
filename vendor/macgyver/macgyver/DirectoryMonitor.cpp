// ======================================================================
/*!
 * \brief Implementation of class DirectoryMonitor
 *
 * The implementation depends heavily on the Schedule object,
 * which essentially consists of a list of monitors sorted
 * by their scheduled update times.
 *
 * If the is no pending check, the run-loop sleeps until the
 * first scheduled update. Waking up, the run-loop checks
 * the scheduled monitors until it runs into a monitor
 * whose scheduled update time is in the future. The time sort
 * is updated after each monitor update, so all the run-loop
 * has to do is keep checking the first scheduled monitor
 * to decide what to do.
 *
 * Upon an observed change, the run-loop calls the listeners
 * sequentially. The listeners may start a new thread to
 * perform any update necessary, the decision is up to the
 * user.
 *
 */
// ======================================================================

#include "DirectoryMonitor.h"
#include "Exception.h"
#include "FileSystem.h"
#include "StringConversion.h"
#include <boost/chrono.hpp>
#include <boost/thread.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/tuple/tuple.hpp>
#include <atomic>
#include <iostream>
#include <optional>
#include <stdexcept>

// scoped read/write lock types

using MutexType = boost::shared_mutex;
using ReadLock = boost::shared_lock<MutexType>;
using WriteLock = boost::unique_lock<MutexType>;

namespace fs = std::filesystem;

namespace Fmi
{
// ----------------------------------------------------------------------
/*!
 * \brief Directory content with modification times
 */
// ----------------------------------------------------------------------

using Contents = std::map<fs::path, std::time_t>;

// ----------------------------------------------------------------------
/*!
 * \brief Scan directory contents
 */
// ----------------------------------------------------------------------

Contents directory_contents(const fs::path& path, bool hasregex, const boost::regex& pattern)
{
  try
  {
    Contents contents;

    // safety against missing dir.
    // in case of single files, the file has been removed

    if (!fs::exists(path))
      return contents;

    // find all matching filenames with modification times
    // if target is a directory

    // Note: files may vanish while being scanned, or be in a corrupted state
    // in which case stat will fail. We simply ignore such cases by checking
    // the error code and do not let last_write_time throw

    if (fs::is_directory(path))
    {
      fs::directory_iterator end;
      for (fs::directory_iterator it(path); it != end; ++it)
      {
        // Check first that the filename does not start with '.'
        char dot = '.';
        if (it->path().filename().native().at(0) != dot)
        {
          std::error_code ec;
          if (hasregex)
          {
            if (boost::regex_match(it->path().filename().string(), pattern))
            {
              const std::time_t t = Fmi::last_write_time(it->path(), ec);
              if (!ec)
                contents.insert(Contents::value_type(it->path(), t));
            }
          }
          else
          {
            const std::time_t t = Fmi::last_write_time(it->path(), ec);
            if (!ec)
              contents.insert(Contents::value_type(it->path(), t));
          }
        }
      }
    }
    else
    {
      // No regex checking for single files
      std::error_code ec;
      const std::time_t t = Fmi::last_write_time(path, ec);
      if (!ec)
        contents.insert(Contents::value_type(path, t));
    }
    return contents;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Scan for changes in a directory
 *
 * Changes:
 * - create = new file created
 * - delete = old file deleted
 * - modify = old file modified
 */
// ----------------------------------------------------------------------

std::pair<DirectoryMonitor::Status, DirectoryMonitor::Change> directory_change(
    const Contents& oldcontents, const Contents& newcontents)
{
  try
  {
    DirectoryMonitor::Status status(new DirectoryMonitor::StatusMap);

    DirectoryMonitor::Change changes = DirectoryMonitor::NONE;

    // Scan old contents detecting modifications and deletes

    for (const Contents::value_type& it : oldcontents)
    {
      const auto pos = newcontents.find(it.first);

      DirectoryMonitor::Change change = DirectoryMonitor::NONE;

      if (pos == newcontents.end())
        change = DirectoryMonitor::DELETE;
      else if (it.second != pos->second)
        change = DirectoryMonitor::MODIFY;

      changes |= change;
      (*status)[it.first] = change;
    }

    // Scan new contents detecting new files

    for (const Contents::value_type& it : newcontents)
    {
      const auto pos = oldcontents.find(it.first);
      if (pos == oldcontents.end())
      {
        changes |= DirectoryMonitor::CREATE;
        (*status)[it.first] = DirectoryMonitor::CREATE;
      }
    }

    return std::make_pair(status, changes);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*
 * \brief Information on a single monitored path/regex
 */
// ----------------------------------------------------------------------

struct Monitor
{
  // static info:
  fs::path path;
  boost::regex pattern;
  bool hasregex;
  int interval;
  DirectoryMonitor::Watcher id;
  DirectoryMonitor::Change mask;
  DirectoryMonitor::Listener callback;
  DirectoryMonitor::ErrorHandler errorhandler;

  // generated data:
  time_t lastmodified;
  Contents contents;
};

// ----------------------------------------------------------------------
/*
 * \brief Information on all monitors
 *
 * The index is the time of the next scheduled update. If the time
 * is zero, it indicates no previous update has been done.
 */
// ----------------------------------------------------------------------

using Schedule = std::multimap<std::time_t, Monitor>;

// ----------------------------------------------------------------------
/*!
 * \brief Implementation interface
 */
// ----------------------------------------------------------------------

class DirectoryMonitor::Pimple
{
 public:
  MutexType mutex;
  Schedule schedule;
  bool running{false};               // true if run() has not exited
  std::atomic<bool> stop{false};     // true if stop request is pending
  std::atomic<bool> isready{false};  // true if at least one scan has completed
  bool has_ended{false};             // true if run() has ended due to any reason

  boost::mutex m2;
  boost::mutex m_ready;
  boost::condition_variable cond;
  boost::condition_variable cond_ready;

  Watcher nextid = 0;
};

// ----------------------------------------------------------------------
/*!
 * \brief Constructor
 */
// ----------------------------------------------------------------------

DirectoryMonitor::DirectoryMonitor() : impl(new Pimple()) {}
// ----------------------------------------------------------------------
/*!
 * \brief Destructor
 */
// ----------------------------------------------------------------------

DirectoryMonitor::~DirectoryMonitor()
{
  if (impl->running)
  {
    std::cout << "[CRITICAL]: Fmi::DirectoryMonitor::~DirectoryMonitor:"
              << " missing call to Fmi::DirectoryMonitor::stop() before destroying object\n"
              << std::flush;
    abort();
  }
}

// ----------------------------------------------------------------------
/*
 * \brief Request a new monitored path
 */
// ----------------------------------------------------------------------

DirectoryMonitor::Watcher DirectoryMonitor::watch(const fs::path& path,
                                                  const boost::regex& pattern,
                                                  Listener callback,
                                                  ErrorHandler errorhandler,
                                                  int interval,
                                                  Change mask)
{
  try
  {
    if (interval < 1)
      throw Fmi::Exception(
          BCP, "DirectoryMonitor: Too small update interval: " + Fmi::to_string(interval));

    if ((mask & ALL) == 0)
      throw Fmi::Exception(BCP, "DirectoryMonitor: Empty mask, nothing to monitor");

    // if(!fs::exists(path))
    //   throw std::runtime_error("DirectoryMonitor: "+path.string()+" does not exist");

    // new monitor

    WriteLock lock(impl->mutex);

    Monitor mon;
    mon.path = path;
    mon.pattern = pattern;
    mon.interval = interval;
    mon.mask = mask;
    mon.callback = callback;
    mon.errorhandler = errorhandler;
    mon.id = impl->nextid;
    mon.lastmodified = 0;
    mon.hasregex = true;

    ++impl->nextid;

    // time_t = 0 implies no update has been made yet by run()

    impl->schedule.insert(std::make_pair(0, mon));

    return mon.id;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*
 * \brief Request a new monitored path
 */
// ----------------------------------------------------------------------

DirectoryMonitor::Watcher DirectoryMonitor::watch(const fs::path& path,
                                                  const std::string& pattern,
                                                  Listener callback,
                                                  ErrorHandler errorhandler,
                                                  int interval,
                                                  Change mask)
{
  try
  {
    return watch(path, boost::regex{pattern}, callback, errorhandler, interval, mask);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*
 * \brief Request a new monitored path without regex
 */
// ----------------------------------------------------------------------

DirectoryMonitor::Watcher DirectoryMonitor::watch(
    const fs::path& path, Listener callback, ErrorHandler errorhandler, int interval, Change mask)
{
  try
  {
    if (interval < 1)
      throw Fmi::Exception(
          BCP, "DirectoryMonitor: Too small update interval: " + Fmi::to_string(interval));

    if ((mask & ALL) == 0)
      throw Fmi::Exception(BCP, "DirectoryMonitor: Empty mask, nothing to monitor");

    // if(!fs::exists(path))
    //   throw std::runtime_error("DirectoryMonitor: "+path.string()+" does not exist");

    // new monitor

    WriteLock lock(impl->mutex);

    Monitor mon;
    mon.path = path;
    mon.interval = interval;
    mon.mask = mask;
    mon.callback = callback;
    mon.errorhandler = errorhandler;
    mon.id = impl->nextid;
    mon.lastmodified = 0;
    mon.hasregex = false;

    ++impl->nextid;

    // time_t = 0 implies no update has been made yet by run()

    impl->schedule.insert(std::make_pair(0, mon));

    return mon.id;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*
 * \brief Start monitoring
 */
// ----------------------------------------------------------------------

void DirectoryMonitor::run()
{
  try
  {
    try
    {
      // Do not start if already running

      do
      {
        boost::unique_lock<boost::mutex> lock(impl->m_ready);
        if (impl->running)
          return;

        impl->running = true;
        impl->has_ended = false;
      } while (false);

      while (!impl->stop && !impl->schedule.empty())
      {
        bool checknext = true;

        while (checknext)
        {
          boost::this_thread::interruption_point();

          WriteLock lock(impl->mutex);
          std::time_t tnow = std::time(nullptr);
          std::time_t tcheck = impl->schedule.begin()->first;

          if (tcheck > tnow)
            checknext = false;
          else
          {
            // pop the monitor from the schedule, insert it back
            // later on with updated information

            Monitor mon = impl->schedule.begin()->second;
            impl->schedule.erase(impl->schedule.begin());

            // first check dir modification time

            // establish nature of changes

            Change changes;
            Status newstatus(new DirectoryMonitor::StatusMap);

            try
            {
              std::optional<std::time_t> tchange;
              if (fs::exists(mon.path))
              {
                tchange = Fmi::last_write_time(mon.path);
                // Actually directory is scanned later if changes detected,
                // but we are interested in how often changes are checked
                if (mon.mask & DirectoryMonitor::SCAN)
                {
                  (*newstatus)[mon.path] = DirectoryMonitor::SCAN;
                  mon.callback(mon.id, mon.path, mon.pattern, newstatus);
                  newstatus->clear();
                }
              }
              else
              {
                tchange = std::time(nullptr);
              }

              // We cannot detect modifications simply by looking
              // at the directory change time. In such cases we
              // must scan the directory contents.

              if (tchange <= mon.lastmodified && !((mon.mask & MODIFY) != 0))
              {
                // nothing to scan since dir timestamp did not change

                std::time_t tnext = std::time(nullptr) + mon.interval;
                impl->schedule.insert(std::make_pair(tnext, mon));
              }
              else
              {
                // new listing must be taken since dir timestamp changed

                Contents newcontents = directory_contents(mon.path, mon.hasregex, mon.pattern);

                boost::tie(newstatus, changes) = directory_change(mon.contents, newcontents);

                // possible callback

                if ((changes & mon.mask) != 0)
                {
                  mon.callback(mon.id, mon.path, mon.pattern, newstatus);
                }

                // update schedule and status

                mon.contents = newcontents;
                mon.lastmodified = tchange ? *tchange : 0;

                std::time_t tnext = std::time(nullptr) + mon.interval;
                impl->schedule.insert(std::make_pair(tnext, mon));
              }
            }
            catch (std::exception& e)
            {
#ifdef DEBUG
              std::cerr << "Warning: " << e.what() << std::endl;
#endif

              if ((mon.mask & ERROR) != 0)
              {
                mon.errorhandler(mon.id, mon.path, mon.pattern, e.what());
              }

              std::time_t tnext = std::time(nullptr) + mon.interval;
              impl->schedule.insert(std::make_pair(tnext, mon));
            }
          }
        }

        {
          boost::unique_lock<boost::mutex> lock(impl->m_ready);
          if (!impl->isready)
          {
            impl->isready = true;
            impl->cond_ready.notify_all();
          }
        }

        long sleeptime = 0;
        {
          ReadLock tmplock(impl->mutex);
          std::time_t tmpnow = std::time(nullptr);
          std::time_t tmpcheck = impl->schedule.begin()->first;
          sleeptime = (tmpnow > tmpcheck ? 0 : tmpcheck - tmpnow);
        }

        if (sleeptime > 0)
        {
          boost::unique_lock<boost::mutex> lock(impl->m2);
          impl->cond.wait_for(
              lock, boost::chrono::seconds(sleeptime), [this]() -> bool { return impl->stop; });
        }
      }

      {
        boost::unique_lock<boost::mutex> lock(impl->m_ready);
        impl->has_ended = true;
        impl->stop = false;
        impl->running = false;
        impl->cond_ready.notify_all();
      }
    }
    catch (const boost::thread_interrupted&)
    {
      impl->running = false;
      throw;
    }
  }
  catch (...)
  {
    boost::unique_lock<boost::mutex> lock(impl->m_ready);
    impl->has_ended = true;
    impl->cond_ready.notify_all();
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*
 * \brief Stop monitoring
 *
 * This should be called from a thread other than the one which called
 * run(), since the exit condition variable in the loop in run() is
 * not changed inside the loop itself.
 */
// ----------------------------------------------------------------------

void DirectoryMonitor::stop()
{
  try
  {
    boost::unique_lock<boost::mutex> lock(impl->m2);
    impl->stop = true;
    impl->cond.notify_all();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Check if at least once scan of all directories has been completed
 */
// ----------------------------------------------------------------------

bool DirectoryMonitor::ready() const
{
  try
  {
    return impl->isready;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

bool DirectoryMonitor::wait_until_ready() const
{
  try
  {
    boost::unique_lock<boost::mutex> lock(impl->m_ready);
    impl->cond_ready.wait(lock, [this]() -> bool { return impl->has_ended || impl->isready; });
    return impl->isready && !impl->has_ended && !impl->stop;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

}  // namespace Fmi

// ======================================================================
