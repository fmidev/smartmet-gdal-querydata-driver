// ======================================================================
/*!
 * \brief Sample program showing how DirectoryMonitor works
 */
// ======================================================================

#include "DirectoryMonitor.h"
#include <boost/bind.hpp>
#include <boost/thread/thread.hpp>

#include <iostream>

#include <signal.h>

using namespace std;
using namespace Fmi;

std::filesystem::path p("tmp");

const int timeout = 30;

void sighandler(int)
{
  cout << "Removing " << p << " after signal catch" << endl;
  std::filesystem::remove_all(p);
  exit(0);
}

void listener(DirectoryMonitor::Watcher id,
              std::filesystem::path dir,
              boost::regex pattern,
              DirectoryMonitor::Status status)
{
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

void errorhandler(DirectoryMonitor::Watcher id,
                  std::filesystem::path dir,
                  boost::regex pattern,
                  std::string message)
{
  cout << "Error: " << message << endl;
}

int main()
{
  signal(SIGINT, &sighandler);
  signal(SIGQUIT, &sighandler);
  signal(SIGHUP, &sighandler);
  signal(SIGABRT, &sighandler);
  signal(SIGTERM, &sighandler);

  std::filesystem::create_directories(p);

  cout << endl
       << "\tThis program has just created a local subfolder named tmp," << endl
       << "\tif it did not already exist." << endl
       << endl
       << "\tPlease create files, delete files etc in there and see" << endl
       << "\twhat happens. When you're done, press Ctrl-C to end the" << endl
       << "\tprogram. The subfolder will be removed before exit." << endl
       << endl
       << "\tYou have " << timeout << " seconds to do your tests before an automatic interrupt."
       << endl
       << endl;

  // We'll run the method in a thread. If the user does not interrupt
  // the program fast enough, we'll abort

  DirectoryMonitor mon;

  mon.watch(p, boost::regex(".*"), &listener, &errorhandler, 1, DirectoryMonitor::ALL);

  // start monitoring in a separate thread
  boost::thread thrd(boost::bind(&DirectoryMonitor::run, &mon));

  thrd.timed_join(boost::posix_time::seconds(timeout));

  cout << endl << "Removing " << p << " after " << timeout << " second timeout" << endl;

  std::filesystem::remove_all(p);

  return 0;
}
