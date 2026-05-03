/*
 * TemplateFormatter.cpp
 *
 *  Created on: Oct 4, 2012
 *      Author: pavenis
 */

#include "TemplateFormatter.h"
#include "Exception.h"

#include <ctpp2/CTPP2Logger.hpp>
#include <ctpp2/CTPP2OutputCollector.hpp>
#include <ctpp2/CTPP2VM.hpp>
#include <ctpp2/CTPP2VMFileLoader.hpp>
#include <ctpp2/CTPP2VMSTDLib.hpp>

#include <cstdarg>
#include <cstdio>
#include <iostream>
#include <string>

class Fmi::TemplateFormatter::OutputCollector : public CTPP::OutputCollector
{
  std::string& out;

 public:
  explicit OutputCollector(std::string& the_out) : out(the_out) {}

  INT_32 Collect(const void* vData, const UINT_32 datalength) override;
};

INT_32 Fmi::TemplateFormatter::OutputCollector::Collect(const void* vData, const UINT_32 datalength)
{
  try
  {
    out.append(reinterpret_cast<const char*>(vData), datalength);
    return 0;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

/**
 *  @brief CTPP logger class for writing log messages to provided std::ostream
 */
class Fmi::TemplateFormatter::Logger : public CTPP::Logger
{
  std::string& out;

 public:
#ifdef __llvm__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"
#endif

  Logger(std::string& the_out, const UINT_32 priority) : CTPP::Logger(priority), out(the_out) {}

#ifdef __llvm__
#pragma clang diagnostic pop
#endif

  INT_32 WriteLog(const UINT_32 priority, CCHAR_P szString, const UINT_32 stringlen) override;
};

INT_32 Fmi::TemplateFormatter::Logger::WriteLog(const UINT_32 priority,
                                                CCHAR_P szString,
                                                const UINT_32 stringlen)
{
  try
  {
    if (priority >= iBasePriority)
    {
      // FIXME: shouldn't reject for too long messages
      out.append(szString, stringlen);
    }

    return 0;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

Fmi::TemplateFormatter::TemplateFormatter(UINT_32 max_handlers) : syscall_factory(max_handlers)
{
  try
  {
    CTPP::STDLibInitializer::InitLibrary(syscall_factory);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

Fmi::TemplateFormatter::~TemplateFormatter()
{
  try
  {
    CTPP::STDLibInitializer::DestroyLibrary(syscall_factory);
  }
  catch (...)
  {
    std::cout << Fmi::Exception(BCP, "ERROR: C++ EXCEPTION IN DESTRUCTOR") << std::endl;
  }
}

int Fmi::TemplateFormatter::process(CTPP::CDT& hash,
                                    std::string& output_string,
                                    std::string& log_string)
{
  try
  {
    return process(hash, output_string, log_string, CTPP2_LOG_WARNING);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

/*
 *
 * CTPP2_LOG_EMERG 0    system is unusable
 * CTPP2_LOG_ALERT 1    action must be taken immediately
 * CTPP2_LOG_CRIT 2     critical conditions
 * CTPP2_LOG_ERR 3      error conditions
 * CTPP2_LOG_WARNING 4  warning conditions
 * CTPP2_LOG_NOTICE 5   normal but significant condition
 * CTPP2_LOG_INFO 6     informational
 * CTPP2_LOG_DEBUG 7    debug-level messages
 */

int Fmi::TemplateFormatter::process(CTPP::CDT& hash,
                                    std::string& output_string,
                                    std::string& log_string,
                                    int log_level)
{
  try
  {
    // Create output collector for provided output stream
    TemplateFormatter::OutputCollector output_collector(output_string);

    // Create virtual machine for template processing
    CTPP::VM tmp_vm(&syscall_factory, 4096, 4096, 0x40000000, log_level);

    // Create logger for writing log messages to the output stream
    TemplateFormatter::Logger logger(log_string, log_level);

    // Get data to run on CTPP2 virtual machine
    const CTPP::VMMemoryCore* p_memory_core = loader->GetCore();

    // Initialize CTPP2 virtual machine for output formatting.
    tmp_vm.Init(p_memory_core, &output_collector, &logger);

    // Finally process the template and generate output
    UINT_32 iIP = 0;
    return tmp_vm.Run(p_memory_core, &output_collector, iIP, hash, &logger);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

int Fmi::TemplateFormatter::process(CTPP::CDT& hash,
                                    std::ostream& output_stream,
                                    std::ostream& log_stream)
{
  try
  {
    return process(hash, output_stream, log_stream, CTPP2_LOG_WARNING);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

int Fmi::TemplateFormatter::process(CTPP::CDT& hash,
                                    std::ostream& output_stream,
                                    std::ostream& log_stream,
                                    int log_level)
{
  try
  {
    std::string output_string;
    std::string log_string;
    auto result = process(hash, output_string, log_string, log_level);
    output_stream << output_string;
    log_stream << log_string;
    return result;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

void Fmi::TemplateFormatter::load_template(const std::string& file_name)
{
  try
  {
    loader.reset(new CTPP::VMFileLoader(file_name.c_str()));
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}
