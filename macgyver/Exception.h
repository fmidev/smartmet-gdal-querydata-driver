#pragma once

#include <atomic>
#include <ctime>
#include <list>
#include <memory>
#include <ostream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace Fmi
{
using ExceptionTimeStamp = std::time_t;
using ParameterVector = std::vector<std::pair<std::string, std::string>>;
using DetailVector = std::vector<std::string>;
using DetailList = std::list<std::string>;

class Exception : public std::exception
{
 public:
  Exception();
  Exception(const Exception& other);

  // Use the following constructor when there is no previous exception in place

  Exception(const char* _filename, int _line, const char* _function, std::string _message);

  // Use the following constructor when there is previous exception in place
  // (i.e when you are in a "catch" block. If '_prevExeption' parameter is nullptr then
  // the constructor automatically detects the content of the previous exception.

  // NOTE: boost::thread_interrupted exceptions are rethrown by this method to allow proper
  //       handling of thread interruptions.
  static Exception Trace(const char* _filename,
                         int _line,
                         const char* _function,
                         std::string _message);

  // Convert std::exception_ptr to Fmi::Exception
  //
  // NOTE: boost::thread_interrupted exceptions are rethrown by this method to allow proper
  //       handling of thread interruptions.
  static Exception Trace(const char* _filename,
                        int _line,
                        const char* _function,
                        std::exception_ptr eptr,
                        std::string _message);

  /**
   *  @brief Returns a copy of deepest level Fmi::Exception object or new one when none is found
   *
   *  Useful when one is not interested in intermediate back-trace levels, but only in
   *  initial exception. One can for example add details and/or parameters to the copy of
   *  initial exception and rethrow it or use for generating error message
   *
   *  NOTE: boost::thread_interrupted exceptions are rethrown by this method to allow proper
   *        handling of thread interruptions.
   */
  static Exception SquashTrace(const char* _filename,
                               int _line,
                               const char* _function,
                               std::string _message);

  // TODO: Make this private to enforce using Exception::Trace
  //
  // NOTE: boost::thread_interrupted exceptions are rethrown by this method to allow proper
  //       handling of thread interruptions.
  Exception(const char* _filename,
            int _line,
            const char* _function,
            std::string _message,
            Exception* _prevException);

  // Destructor
  ~Exception() override;

  // The following methods can be used for adding some additional information
  // related to the current exception.

  Exception& addDetail(std::string _detailStr);
  Exception& addDetails(const DetailList& _detailList);

  // This method can be used for adding named parameters into the exception.
  // The parameters can be "incorrect" values that caused the exception. They
  // can be also used for delivering additional information to the exception
  // catchers ("preferred HTTP status code",etc.).

  Exception& addParameter(const char* _name, std::string _value);

  const char* what() const noexcept(true) override;
  const char* getWhat() const noexcept(true);

  std::string getFilename() const;
  int getLine() const;
  std::string getFunction() const;

  const Exception* getFirstException() const;
  const Exception* getExceptionByIndex(unsigned int _index) const;
  const Exception* getExceptionByParameterName(const char* _paramName) const;
  const Exception* getPrevException() const;

  unsigned int getExceptionCount() const;
  unsigned int getDetailCount() const;
  unsigned int getParameterCount() const;

  const char* getDetailByIndex(unsigned int _index) const;
  const char* getParameterNameByIndex(unsigned int _index) const;
  const char* getParameterValue(const char* _name) const;
  const char* getParameterValueByIndex(unsigned int _index) const;

  ExceptionTimeStamp getTimeStamp() const;
  void setTimeStamp(ExceptionTimeStamp _timestamp);
  std::string getTimeStampString() const;

  std::string getStackTrace() const;
  std::string getHtmlStackTrace() const;

  bool loggingDisabled() const;
  bool stackTraceDisabled() const;

  Exception& disableLogging();
  Exception& disableStackTrace();
  Exception& disableStackTraceRecursive();

  void printError() const;

  void printOn(std::ostream& out) const;

 public:
  class ForceStackTrace final
  {
  public:
      ForceStackTrace();
      ForceStackTrace(const ForceStackTrace&) = delete;
      ~ForceStackTrace();
      ForceStackTrace& operator = (const ForceStackTrace&) = delete;
  private:
      bool prev;
  };

  /**
   * @brief Force outputting stack trace for the all threads
   *
   * This is a global setting that affects all threads. It is useful for debugging purposes only.
   * It is not recommended to use this in production code.
   */
  static bool global_force_stack_trace;

 protected:
  ExceptionTimeStamp timestamp;
  std::string filename;
  int line;
  std::string function;
  std::string message;
  std::shared_ptr<Exception> prevException;
  ParameterVector parameterVector;
  DetailVector detailVector;
  bool mLoggingDisabled = false;
  bool mStackTraceDisabled = false;

  static thread_local bool force_stack_trace;
};

std::ostream& operator << (std::ostream& out, const Exception& e);

/**
 *  @brief Intended to be used in catch block to ignore exceptions thrown except some predefined ones
 *
 *  Following exceptions are thrown again:
 *    - boost::thread_interrupted - required for support of boost::thread::interrupt (so also
 *                for Fmi::AsyncTask)
 *
 *  Notes:
 *    - Fmi::Exception::Trace already provides current handling of boost::thread_interrupted
 *    - using empty catch block interferes with boost::thread::interrupt - use this method in throw
 *      block instead of leaving it empty
 */
void ignore_exceptions();

// Next is to be replaced later on with std::source_location, which is currently experimental
// Static cast explanation: https://github.com/isocpp/CppCoreGuidelines/issues/765

#ifndef BCP
#define BCP __FILE__, __LINE__, static_cast<const char*>(__PRETTY_FUNCTION__)
#endif

}  // namespace Fmi
