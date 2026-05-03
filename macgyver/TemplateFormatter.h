/*
 * TemplateFormatter.h
 */

#pragma once

#include <boost/scoped_ptr.hpp>
#include <ctpp2/CDT.hpp>
#include <ctpp2/CTPP2SyscallFactory.hpp>

namespace CTPP
{
// No need to expose details of several CTPP classes to the user
class VMLoader;
class VM;
}  // namespace CTPP

namespace Fmi
{
/**
 *   @brief Simple CTTP2 library based output formatter
 *
 *   Currently only template loading from files is supported
 *
 *   Note that it is not safe to use the same TemplateFormatter
 *   simultaneously from different threads. Distinct objects
 *   can safely be used simultaneously from different threads.
 */
class TemplateFormatter
{
  class OutputCollector;
  class Logger;

 public:
  TemplateFormatter(UINT_32 max_handlers = 100);

  virtual ~TemplateFormatter();

  TemplateFormatter(const TemplateFormatter& other) = delete;
  TemplateFormatter(TemplateFormatter&& other) = delete;
  TemplateFormatter& operator=(const TemplateFormatter& other) = delete;
  TemplateFormatter& operator=(TemplateFormatter&& other) = delete;

  /**
   *  @brief Generate an output data from template on the base of provided values in the hash
   *
   *  @param hash A hash with values to use for generating output
   *  @param output_string an output string where to write formatter output to.
   *  @param log_string an output string where to write formatter log messages to.
   *  @return the value returned by CTPP::VM::Run()
   */
  int process(CTPP::CDT& hash, std::string& output_string, std::string& log_string);

  /**
   *  @brief Generate an output data from template on the base of provided values in the hash
   *
   *  @param hash A hash with values to use for generating output
   *  @param output_string an output string where to write formatter output to.
   *  @param log_string an output string where to write formatter log messages to.
   *  @param log_level the logging level.
   *  @return the value returned by CTPP::VM::Run()
   */
  int process(CTPP::CDT& hash, std::string& output_string, std::string& log_string, int log_level);

  /**
   *  @brief Generate an output data from template on the base of provided values in the hash
   *
   *  @param hash A hash with values to use for generating output
   *  @param output_stream an output stream where to write formatter output to.
   *  @param log_stream an output stream where to write formatter log messages to.
   *  @return the value returned by CTPP::VM::Run()
   */
  int process(CTPP::CDT& hash, std::ostream& output_stream, std::ostream& log_stream);

  /**
   *  @brief Generate an output data from template on the base of provided values in the hash
   *
   *  @param hash A hash with values to use for generating output
   *  @param output_stream an output stream where to write formatter output to.
   *  @param log_stream an output stream where to write formatter log messages to.
   *  @param log_level the logging level.
   *  @return the value returned by CTPP::VM::Run()
   */
  int process(CTPP::CDT& hash,
              std::ostream& output_stream,
              std::ostream& log_stream,
              int log_level);

  /**
   *  @brief Load a template for output generation
   *
   *  @param file_name a name of template file to load
   *
   *  The file is expected to be output of command @b ctpp2c which compiles template
   *  to the internal format
   *
   *  An exception derived from CTPP::CTPException (derived from std::exception) is
   *  thrown if an error occurs while loading the template.
   */
  void load_template(const std::string& file_name);

 private:
  CTPP::SyscallFactory syscall_factory;
  boost::scoped_ptr<CTPP::VMLoader> loader;
  boost::scoped_ptr<CTPP::VM> vm;
};

} /* namespace Fmi */
