// ======================================================================
/*!
 * \brief Sample program showing how DirectoryMonitor works.
 *
 * This differs from the monitor example in that here the callback
 * function is a method, not a regular function.
 *
 */
// ======================================================================

#include "DirectoryMonitor.h"
#include <boost/bind.hpp>
#include <boost/thread/thread.hpp>

#include <iostream>

extern "C"
{
#include <signal.h>
}

using namespace std;
using namespace Fmi;

const int timeout = 60 * 5;  // 5 minutes

class Handler
{
 private:
  DirectoryMonitor mon;

 public:
  void run() { mon.run(); }

  // Can't get this to work:
  void sighandler(int x) const
  {
    cout << "Exiting after signal catch" << endl;
    exit(0);
  }

  void errorhandler(DirectoryMonitor::Watcher id,
                    std::filesystem::path dir,
                    boost::regex pattern,
                    std::string message) const
  {
    cout << "Monitor " << id << " errored: " << message << endl;
  }

  void listener(DirectoryMonitor::Watcher id,
                std::filesystem::path dir,
                boost::regex pattern,
                DirectoryMonitor::Status status) const
  {
    cout << "Trigger " << id << " on " << dir << ":" << endl;

    for (const DirectoryMonitor::StatusMap::value_type& it : *status)
    {
      switch (it.second)
      {
        case DirectoryMonitor::CREATE:
          cout << it.first << " created" << endl;
          break;
        case DirectoryMonitor::DELETE:
          cout << it.first << " deleted" << endl;
          break;
        case DirectoryMonitor::MODIFY:
          cout << it.first << " modified" << endl;
          break;
      }
    }
  }

  void add_watches(const std::filesystem::path& dir)
  {
    if (!std::filesystem::exists(dir))
      throw std::runtime_error("Directory " + dir.string() + " does not exist");
    if (!std::filesystem::is_directory(dir))
      throw std::runtime_error(dir.string() + " is not a directory");

    std::filesystem::directory_iterator end;
    for (std::filesystem::directory_iterator it(dir); it != end; ++it)
    {
      std::string trig = it->path().filename().string();
      std::replace(trig.begin(), trig.end(), ':', '/');
      trig = '/' + trig;
      if (std::filesystem::exists(trig) && std::filesystem::is_directory(trig))
      {
        // we omit modify since it require full directory polling
        // delete is omitted since it does not trigger action
        mon.watch(trig,
                  boost::regex(".*"),
                  boost::bind(&Handler::listener, this, _1, _2, _3, _4),
                  boost::bind(&Handler::errorhandler, this, _1, _2, _3, _4),
                  5,
                  // DirectoryMonitor::DELETE |
                  // DirectoryMonitor::MODIFY |
                  DirectoryMonitor::CREATE);
      }
    }
  }
};

void sighandler(int)
{
  cout << "exiting after signal catch" << endl;
  exit(0);
}

void sighandler2(const Handler& handler, int x) { handler.sighandler(x); }

int main()
{
  try
  {
    Handler handler;

#if 0
	  // Can't get either one to work:
	  signal(SIGINT, boost::bind(&Handler::sighandler,&handler,_1));
	  signal(SIGINT ,boost::bind(&sighandler2,boost::cref(handler),_1));
#else
    signal(SIGINT, &sighandler);
#endif
    signal(SIGQUIT, &sighandler);
    signal(SIGHUP, &sighandler);
    signal(SIGABRT, &sighandler);
    signal(SIGTERM, &sighandler);

    cout << endl
         << "\tThis program will monitor the triggers for a while." << endl
         << "\tYou can abort the program by pressing Ctrl-C." << endl
         << endl
         << "\tThere are " << timeout << " seconds before the program will" << endl
         << "\texit automatically." << endl
         << endl;

    // We'll run the method in a thread. If the user does not interrupt
    // the program fast enough, we'll abort

    cout << "Scanning quick triggers" << endl;
    handler.add_watches("/smartmet/cnf/triggers.d/quick");

    cout << "Scanning lazy triggers" << endl;
    handler.add_watches("/smartmet/cnf/triggers.d/lazy");

    boost::thread thrd(boost::bind(&Handler::run, &handler));

    thrd.timed_join(boost::posix_time::seconds(timeout));

    cout << endl << "Ending the program after a " << timeout << " second timeout" << endl;
  }
  catch (std::exception& e)
  {
    cerr << "Exception: " << e.what();
    return 1;
  }

  return 0;
}
