// ======================================================================
/*!
 * \brief Directory change monitor
 */
// ======================================================================

#pragma once

#include <boost/regex.hpp>
#include <ctime>
#include <filesystem>
#include <functional>
#include <map>
#include <memory>
#include <string>

namespace Fmi
{
class DirectoryMonitor
{
 public:
  // Watcher ID

  using Watcher = std::size_t;

  // Change type

  using Change = unsigned int;

  // Observable events

  static const Change NONE = 0x00;    // no changes
  static const Change CREATE = 0x01;  // new file created (or first pass)
  static const Change DELETE = 0x02;  // old file deleted
  static const Change MODIFY = 0x04;  // old file modified
  static const Change SCAN = 0x08;    // directory scanned
  static const Change ERROR = 0x10;   // error occured
  static const Change ALL = 0x1F;     // any change

  // Directory listing with modification state

  using StatusMap = std::map<std::filesystem::path, Change>;
  using Status = std::shared_ptr<StatusMap>;

  DirectoryMonitor();
  ~DirectoryMonitor();

  DirectoryMonitor(const DirectoryMonitor& other) = delete;
  DirectoryMonitor& operator=(const DirectoryMonitor& other) = delete;
  DirectoryMonitor(DirectoryMonitor&& other) = delete;
  DirectoryMonitor& operator=(DirectoryMonitor&& other) = delete;

  // Callback interfaces. Note that const references could be used
  // since a write lock exists during the callback. However, if
  // the callback instantiates a new thread and passes the
  // referenced variables on to the new thread, new copies should
  // be made. Since we cannot enforce such code in this API,
  // overall it is safer just to make new copies directly
  // in the callback. Callbacks do not occur that often, so the
  // overhead is reasonably small compared to possible damage.

  // Can we actually enforce the API here? What if the called API
  // actually declares references instead of this API? A simple
  // test shows that the code compiles if the callee declares
  // references instead of copies.

  using Listener = std::function<void(Watcher id,
                                        const std::filesystem::path& path,
                                        const boost::regex& pattern,
                                        const Status& status)>;

  using ErrorHandler = std::function<void(Watcher id,
                                            const std::filesystem::path& path,
                                            const boost::regex& pattern,
                                            const std::string& message)>;

  // Request new monitored path/regex

  Watcher watch(const std::filesystem::path& path,
                const boost::regex& pattern,
                Listener callback,
                ErrorHandler errorhandler,
                int interval = 60,
                Change mask = ALL);

  Watcher watch(const std::filesystem::path& path,
                const std::string& pattern,
                Listener callback,
                ErrorHandler errorhandler,
                int interval = 60,
                Change mask = ALL);

  // Request new monitored path, without regex
  Watcher watch(const std::filesystem::path& path,
                Listener callback,
                ErrorHandler errorhandler,
                int interval = 60,
                Change mask = ALL);

  // Start monitoring
  void run();

  // Stop monitoring
  void stop();

  // Return true if at least once scan has been completed
  bool ready() const;

  // Returns when first scanning of directory is done or method run() has ended for any reason
  bool wait_until_ready() const;

 private:
  class Pimple;
  std::unique_ptr<Pimple> impl;

};  // class DirectoryMonitor
}  // namespace Fmi

// ======================================================================
