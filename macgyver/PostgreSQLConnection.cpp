#include "PostgreSQLConnection.h"
#include "PostgreSQLConnectionImpl.h"
#include "AsyncTask.h"
#include "Exception.h"
#include "NumericCast.h"
#include "StringConversion.h"
#include "TypeName.h"
#include <boost/algorithm/string.hpp>
#include <boost/thread.hpp>
#include <fmt/format.h>
#include <algorithm>
#include <cassert>
#include <chrono>
#include <iostream>
#include <optional>
#include <sstream>
#include <variant>
#include <vector>

namespace ba = boost::algorithm;

namespace Fmi
{
namespace Database
{
namespace
{
using uint_member_ptr = unsigned int PostgreSQLConnectionOptions::*;
using string_member_ptr = std::string PostgreSQLConnectionOptions::*;

constexpr std::monostate ignore;

const std::map<std::string, std::variant<std::monostate, uint_member_ptr, string_member_ptr> >
    field_def = {{"host", &PostgreSQLConnectionOptions::host},
                 {"dbname", &PostgreSQLConnectionOptions::database},
                 {"port", &PostgreSQLConnectionOptions::port},
                 {"user", &PostgreSQLConnectionOptions::username},
                 {"password", &PostgreSQLConnectionOptions::password},
                 {"client_encoding", &PostgreSQLConnectionOptions::encoding},
                 {"connect_timeout", &PostgreSQLConnectionOptions::connect_timeout}
                 // FIXME: add parameters we ignore
                 ,
                 {"options", ignore}};
}  // namespace

// ----------------------------------------------------------------------

std::atomic<bool> PostgreSQLConnection::shuttingDown(false);
std::atomic<bool> PostgreSQLConnection::reconnectDisabled(false);
std::size_t PostgreSQLConnection::queryRetryLimit(2);

PostgreSQLConnectionOptions::PostgreSQLConnectionOptions(const std::string& conn_str)
{
  try
  {
    std::vector<std::string> parts;
    ba::split(parts, conn_str, ba::is_any_of(" \t"), ba::token_compress_on);
    for (const std::string& part : parts)
    {
      const std::size_t p = part.find('=');
      if (p == std::string::npos)
        throw Fmi::Exception(BCP, "Unrecognized part '" + part + "'");

      const std::string name = part.substr(0, p);
      const std::string value = part.substr(p + 1);
      const auto it = field_def.find(name);
      if (it == field_def.end())
      {
        throw Fmi::Exception(BCP, "Unrecognized field '" + part + "'");
      }

      switch (it->second.index())
      {
        case 0:
          // std::cout << METHOD_NAME << ": field '" << part << "' ignored" << std::endl;
          break;
        case 1:
          this->*std::get<uint_member_ptr>(it->second) =
              Fmi::numeric_cast<unsigned int>(Fmi::stoul(value));
          break;
        case 2:
          this->*std::get<string_member_ptr>(it->second) = value;
          break;

        default:  // Not supposed to be here
          assert("Not supposed to be here");
      }
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Failed to parse connection string '" + conn_str + "'");
  }
}

PostgreSQLConnectionOptions::operator std::string() const
{
  std::ostringstream ss;

  // clang-format off
  ss << "host="      << host
     << " dbname="   << database
     << " port="    << port
     << " user="     << username
     << " password=" << password
#if 0
     << " client_encoding=" << encoding
#endif
       ;
  // clang-format on

  if (connect_timeout > 0)
    ss << " connect_timeout=" << connect_timeout;

  return ss.str();
}


// ----------------------------------------------------------------------

PostgreSQLConnection::PostgreSQLConnection(const PostgreSQLConnectionOptions& theConnectionOptions)
    : impl(new Impl(theConnectionOptions))
{
}

bool PostgreSQLConnection::open(const PostgreSQLConnectionOptions& theConnectionOptions)
{
  try
  {
    return bool(impl->open(theConnectionOptions));
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

bool PostgreSQLConnection::reopen() const
{
  try
  {
    return bool(impl->reopen());
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

void PostgreSQLConnection::close() const
{
  try
  {
    impl->close();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

PostgreSQLConnectionId PostgreSQLConnection::getId() const
{
  return impl->getId();
}

std::string PostgreSQLConnection::quote(const std::string& theString) const
{
  try
  {
    return impl->quote(theString);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

pqxx::result PostgreSQLConnection::executeNonTransaction(const std::string& theSQLStatement) const
{
  try
  {
    return impl->executeNonTransaction(theSQLStatement, pqxx::params{});
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

pqxx::result PostgreSQLConnection::execute(const std::string& theSQLStatement) const
{
  try
  {
    return impl->execute(false, theSQLStatement, pqxx::params{});
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}


pqxx::result PostgreSQLConnection::exec_params_impl(const std::string& theSQLStatement, const pqxx::params& params) const
try
{
  AsyncTask::interruption_point();
  return impl->exec_params(false, theSQLStatement, params);
}
catch (...)
{
  auto error = Fmi::Exception::Trace(BCP, "Operation failed!");
  error.addParameter("SQL statement", theSQLStatement);
  error.addParameter("Database address", fmt::format("{}", getId()));
  throw error;
}


template <>
pqxx::result PostgreSQLConnection::exec_params(
    const std::string& theSQLStatement,
    const pqxx::params& params) const
try
{
  AsyncTask::interruption_point();
  return impl->exec_params(false, theSQLStatement, params);
}
catch (...)
{
  auto error = Fmi::Exception::Trace(BCP, "Operation failed!");
  error.addParameter("SQL statement", theSQLStatement);
  error.addParameter("Database address", fmt::format("{}", getId()));
  throw error;
}



PostgreSQLConnection::PreparedSQL::Ptr PostgreSQLConnection::prepare(
    const std::string& name, const std::string& theSQLStatement) const
{
  return std::make_shared<PreparedSQL>(*this, name, theSQLStatement);
}


pqxx::result PostgreSQLConnection::exec_prepared(const std::string& name, pqxx::params params) const
try
{
  return impl->exec_prepared(false, name, params);
}
catch(...)
{
  auto error = Fmi::Exception::Trace(BCP, "Operation failed!");
  error.addParameter("Prepared SQL statement name", name);
  error.addParameter("Database address", fmt::format("{}", getId()));
  throw error;
}


void PostgreSQLConnection::cancel()
{
  try
  {
    impl->cancel();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Transaction cancellation failed!");
  }
}

void PostgreSQLConnection::setClientEncoding(const std::string& theEncoding)
{
  try
  {
    impl->setClientEncoding(theEncoding);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

PostgreSQLConnection::PostgreSQLConnection(bool theDebug) : impl(new Impl(theDebug)) {}

bool PostgreSQLConnection::isConnected() const
{
  return impl->isConnected();
}
void PostgreSQLConnection::setDebug(bool debug)
{
  impl->setDebug(debug);
}

bool PostgreSQLConnection::isDebug() const
{
  return impl->isDebug();
}

bool PostgreSQLConnection::collateSupported() const
{
  return impl->collateSupported();
}

std::shared_ptr<pqxx::transaction_base> PostgreSQLConnection::get_transaction_impl() const
{
  return impl->get_transaction_impl();
}

const std::map<unsigned int, std::string>& PostgreSQLConnection::dataTypes() const
{
  return impl->dataTypes();
}

std::shared_ptr<PostgreSQLConnection::Transaction> PostgreSQLConnection::transaction() const
{
  return std::make_shared<PostgreSQLConnection::Transaction>(*this);
}

bool PostgreSQLConnection::isTransaction() const
{
  return impl->isTransaction();
}

void PostgreSQLConnection::startTransaction() const
{
  impl->startTransaction();
}

void PostgreSQLConnection::endTransaction() const
{
  impl->endTransaction();
}

void PostgreSQLConnection::commitTransaction() const
{
  impl->commitTransaction();
}

PostgreSQLConnection::~PostgreSQLConnection() = default;

// ----------------------------------------------------------------------

PostgreSQLConnection::Transaction::Transaction(const PostgreSQLConnection& theConnection)
    : conn(theConnection)
{
  try
  {
    if (conn.isTransaction())
      throw Fmi::Exception(BCP, "Recursive transactions are not supported");

    conn.startTransaction();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

PostgreSQLConnection::Transaction::~Transaction()
{
  try
  {
    conn.endTransaction();
  }
  catch (...)
  {
  }
}

pqxx::result PostgreSQLConnection::Transaction::execute(const std::string& theSQLStatement) const
{
  try
  {
    if (conn.isTransaction())
    {
      if (conn.isDebug())
      {
        std::cout << "SQL: " << theSQLStatement << std::endl;
      }
      return conn.execute(theSQLStatement);
    }

    throw Fmi::Exception(BCP, "[Logic error] Called after transaction commit or rollback");
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

template <>
pqxx::result PostgreSQLConnection::Transaction::exec_params(
      const std::string& theSQLStatement,
      const pqxx::params& params) const
try
{
  return conn.exec_params(theSQLStatement, params);
}
catch (...)
{
  auto error = Fmi::Exception::Trace(BCP, "Operation failed!");
  error.addParameter("SQL statement", theSQLStatement);
  error.addParameter("Database address", fmt::format("{}", conn.getId()));
  throw error;
}


template <>
pqxx::result PostgreSQLConnection::Transaction::exec_prepared(
    const std::string& name,
    const pqxx::params& params) const
{
  try
  {
    return conn.impl->exec_prepared(true, name, params);
  }
  catch (...)
  {
    auto error = Fmi::Exception::Trace(BCP, "Operation failed!");
    error.addParameter("Prepared SQL statement name", name);
    error.addParameter("Database address", fmt::format("{}", conn.getId()));
    throw error;
  }
}



void PostgreSQLConnection::Transaction::commit()
{
  try
  {
    if (conn.isTransaction())
    {
      try
      {
        conn.commitTransaction();
        conn.endTransaction();
      }
      catch (const std::exception& e)
      {
        // If we get here, transaction has been rolled back
        conn.endTransaction();
        throw Fmi::Exception(BCP, "Commiting transaction failed").addDetail(e.what());
      }
    }
    else
    {
      throw Fmi::Exception(BCP,
                           "[Logic error] No transaction to commit"
                           " (already commited or rolled back)");
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

void PostgreSQLConnection::Transaction::rollback()
{
  try
  {
    if (conn.isTransaction())
    {
      conn.endTransaction();
    }
    else
    {
      throw Fmi::Exception(BCP,
                           "[Logic error] No transaction to roll back"
                           " (already commited or rolled back)");
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

PostgreSQLConnection::PreparedSQL::PreparedSQL(const PostgreSQLConnection& theConnection,
                                               const std::string& name,
                                               const std::string& theSQLStatement)

    : conn(theConnection), name(name), sql(theSQLStatement)
{
  try
  {
    conn.impl->register_prepared_statement(name, theSQLStatement);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

PostgreSQLConnection::PreparedSQL::~PreparedSQL()
{
  try
  {
    conn.impl->unregister_prepared_statement(name);
  }
  catch (...)
  {
    // We are not interested about errors when unpreparing SQL (eg. not found)
  }
}


template <>
pqxx::result PostgreSQLConnection::PreparedSQL::exec(pqxx::params params) const
{
  try
  {
    return conn.impl->exec_prepared(false, name, params);
  }
  catch (...)
  {
    auto error = Fmi::Exception::Trace(BCP, "Operation failed!");
    error.addParameter("Prepared SQL statement name", name);
    error.addParameter("Database address", fmt::format("{}", conn.getId()));
    throw error;
  }
}

}  // namespace Database
}  // namespace Fmi

// ======================================================================
