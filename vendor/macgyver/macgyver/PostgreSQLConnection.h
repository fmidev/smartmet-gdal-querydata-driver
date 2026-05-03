// ======================================================================
/*!
 * \brief Interface of class Fmi::Connection
 */
// ======================================================================

#pragma once

#include <atomic>
#include <functional>
#include <memory>
#include <fmt/format.h>
#include <pqxx/pqxx>
#include <string>
#include "Exception.h"
#include "Pool.h"
#include "TypeTraits.h"


namespace Fmi
{
namespace Database
{

/**
 *  @brief Identifier for a PostgreSQL connection (useful for logging)
 */
struct PostgreSQLConnectionId
{
  std::string host;
  unsigned int port;
  std::string database;
};

struct PostgreSQLConnectionOptions
{
  std::string host;
  unsigned int port = 5432;
  std::string database;
  std::string username;
  std::string password;
  std::string encoding = "UTF8";
  unsigned int connect_timeout = 0;
  unsigned int slow_query_limit = 0; // seconds
  bool debug = false;

  PostgreSQLConnectionOptions() = default;
  PostgreSQLConnectionOptions(const std::string& conn_str);

  operator std::string() const;

  inline PostgreSQLConnectionId getId() const { return {host, port, database}; }
};

class PostgreSQLConnection
{
 public:
  class Transaction final
  {
  public:
    ~Transaction();
    Transaction(const PostgreSQLConnection& theConnection);

    Transaction() = delete;
    Transaction(const Transaction& other) = delete;
    Transaction& operator=(const Transaction& other) = delete;
    Transaction(Transaction&& other) = delete;
    Transaction& operator=(Transaction&& other) = delete;

    pqxx::result execute(const std::string& theSQLStatement) const;

    template <typename... Args>
    pqxx::result exec_params(const std::string& theSQLStatement, Args... args) const
    {
      pqxx::params params(std::forward<Args>(args)...);
      return conn.exec_params(theSQLStatement, params);
    }

    template <typename... Args>
    pqxx::result exec_prepared(const std::string& name, Args... args) const
    try
    {
      pqxx::params params(std::forward<Args>(args)...);
      return exec_prepared(name, params);
    }
    catch (...)
    {
      throw Fmi::Exception::Trace(BCP, "Operation failed!");
    }

    void commit();
    void rollback();

   private:
    const PostgreSQLConnection& conn;
  };

  class PreparedSQL final
  {
      const PostgreSQLConnection& conn;
      const std::string name;
      const std::string sql;
   public:
      PreparedSQL(const PostgreSQLConnection& theConnection, const std::string& name, const std::string& sql);
      ~PreparedSQL();

      /**
       *   @brief execute prepared SQL statement with parameters provided in a container
       *         (eg. std::vector)
       *
       *   @param container container with parameters
       *   @param requested_size optional requested row count in the response (default unlimited)
       */
      template <typename Container>
      typename std::enable_if<is_iterable<Container>::value, pqxx::result>::type
      exec_p(const Container& container, std::optional<unsigned> requested_size = std::nullopt) const;


      /**
       *  @brief Execute prepared SQL statement with parameters provided in a container
       *         and expect certain number of rows in the response
       */
      template <typename... Args> pqxx::result exec_n(unsigned nRows, Args... args) const;

      /**
       *  @brief Execute prepared SQL statement with parameters provided in a container
       */
      template <typename... Args> pqxx::result exec(Args... args) const;

      using Ptr = std::shared_ptr<PreparedSQL>;
  };

  ~PostgreSQLConnection();
  PostgreSQLConnection(bool theDebug = false);
  PostgreSQLConnection(const PostgreSQLConnectionOptions& theConnectionOptions);

  bool open(const PostgreSQLConnectionOptions& theConnectionOptions);
  bool reopen() const;

  void close() const;
  PostgreSQLConnectionId getId() const;
  bool isConnected() const;
  void setClientEncoding(const std::string& theEncoding);
  bool isDebug() const;
  void setDebug(bool debug);
  pqxx::result executeNonTransaction(const std::string& theSQLStatement) const;

  /**
   *   @brief Execute SQL statement in current transaction or call executeNonTransaction if no
   *          transaction is active
   */
  pqxx::result execute(const std::string& theSQLStatement) const;
  void cancel();

  PreparedSQL::Ptr prepare(const std::string& name, const std::string& theSQLStatement) const;

  template <typename... Args>
  pqxx::result exec_params(const std::string& theSQLStatement, Args... args) const
  {
     pqxx::params params(std::forward<Args>(args)...);
     return exec_params_impl(theSQLStatement, params);
  }

  template <typename... Args>
  pqxx::result exec_params_n(std::size_t num_rows, const std::string& theSQLStatement,
      Args... args) const
  {
     pqxx::params params(std::forward<Args>(args)...);
     return exec_params_n(num_rows, theSQLStatement, params);
  }

  /**
   *   @brief execute prepared SQL statement with parameters provided in a container
   *         (eg. std::vector)
   *
   *   @param container container with parameters
   *   @param requested_size requested row count in the response
   *                         (default -1, negative value means no limits)
   */
  template <typename Container>
  typename std::enable_if<is_iterable<Container>::value, pqxx::result>::type
  exec_params_p(
      const std::string& theSQLStatement,
      const Container& container,
      int requested_size =  -1) const;

  pqxx::result exec_prepared(const std::string& name, pqxx::params params) const;

  bool collateSupported() const;
  std::string quote(const std::string& theString) const;
  const std::map<unsigned int, std::string>& dataTypes() const;

  std::shared_ptr<Transaction> transaction() const;

  /**
   *   @brief Notifies all PostgreSQL objects that application shutdown is in progress
   *
   *   Note that this call cannot be undone without restarting application
   */
  static void shutdownAll() { shuttingDown.store(true); }

  /**
   *   @brief Disable reconnect attempts for all PostgreSQL objects (for use in tests)
   */
    static void disableReconnect() { reconnectDisabled.store(true); }

 private:
  bool isTransaction() const;
  void startTransaction() const;
  void endTransaction() const;
  void commitTransaction() const;

  pqxx::result exec_params_impl(
      const std::string& theSQLStatement,
      const pqxx::params& params) const;

  std::shared_ptr<pqxx::transaction_base> get_transaction_impl() const;

  pqxx::work get_work() const;

  class Impl;
  std::unique_ptr<Impl> impl;

  static std::atomic<bool> shuttingDown;
  static std::atomic<bool> reconnectDisabled;
  static std::size_t queryRetryLimit;
};  // class PostgreSQLConnection

}  // namespace Database
}  // namespace Fmi

namespace fmt
{

/**
 * @brief Custom formatter for Fmi::Database::PostgreSQLConnectionId
 *
 * Usage:
 *   PostgreSQLConnectionId id{"db.example.com", 5432, "example"};
 *   fmt::format("{}", id)      -> "db.example.com:5432/example"
 *   fmt::format("{:h}", id)    -> "db.example.com"
 *   fmt::format("{:p}", id)    -> "5432"
 *   fmt::format("{:d}", id)    -> "example"
 *
 * Based on verson suggested by ChatGPT 5 with some simplifications
 */
template <>
struct formatter<Fmi::Database::PostgreSQLConnectionId>
{
  enum class part { all, host, port, db } which = part::all;

    // Parse "{:h}", "{:p}", "{:d}", or "{}"
  constexpr auto parse(format_parse_context& ctx) -> format_parse_context::iterator {
    auto it = ctx.begin(), end = ctx.end();
    if (it == end || *it == '}') return it;  // default: all

    switch (*it) {
      case 'h': which = part::host; break;
      case 'p': which = part::port; break;
      case 'd': which = part::db;   break;
      default:  which = part::all;  break;   // anything else -> default
    }
    ++it;
    // Ignore anything until '}' (we don't support other specs)
    while (it != end && *it != '}') ++it;
    return it;
  }

  template <class FormatContext>
  auto format(const Fmi::Database::PostgreSQLConnectionId& v, FormatContext& ctx) const
      -> decltype(ctx.out())
  {
    switch (which) {
    case part::host: return fmt::format_to(ctx.out(), "{}", v.host);
    case part::port: return fmt::format_to(ctx.out(), "{}", v.port);
    case part::db:   return fmt::format_to(ctx.out(), "{}", v.database);
    case part::all:
    default:
      return fmt::format_to(ctx.out(), "{}:{}/{}", v.host, v.port, v.database);
    }
  }
};

}  // namespace fmt

namespace Fmi
{
namespace Database
{

template <>
pqxx::result PostgreSQLConnection::Transaction::exec_prepared(
  const std::string& name,
  const pqxx::params& params) const;


template <typename Container>
typename std::enable_if<is_iterable<Container>::value, pqxx::result>::type
PostgreSQLConnection::PreparedSQL::exec_p(
  const Container& container,
  std::optional<unsigned> requested_size) const
{
  try
  {
    pqxx::params params;
    params.append_multi(container);
    pqxx::result result = conn.exec_prepared(name, params);
    if (requested_size && unsigned(result.size()) != *requested_size)
    {
      throw Fmi::Exception(BCP,
          fmt::format("Expected {} rows but got {} rows", *requested_size, result.size()));
    }
    return result;
  }
  catch (...)
  {
    auto error = Fmi::Exception::Trace(BCP, "Operation failed!");
    error.addParameter("Prepared SQL statement name", name);
    error.addParameter("Database address", fmt::format("{}", conn.getId()));
    throw error;
  }
}


template <typename... Args>
pqxx::result PostgreSQLConnection::PreparedSQL::exec_n(unsigned nRows, Args... args) const
try
{
  auto result = exec(std::forward<Args>(args)...);
  result.expect_size(nRows);
  return result;
}
catch (...)
{
  throw Fmi::Exception::Trace(BCP, "Operation failed!");
}


template <typename... Args>
pqxx::result PostgreSQLConnection::PreparedSQL::exec(Args... args) const
try
{
  auto transaction_ptr = conn.get_transaction_impl();
  return transaction_ptr->exec_prepared(name, args...);
}
catch (...)
{
  throw Fmi::Exception::Trace(BCP, "Operation failed!");
}


template <>
pqxx::result PostgreSQLConnection::exec_params(const std::string& theSQLStatement, const pqxx::params& params) const;


template < typename Container>
typename std::enable_if<is_iterable<Container>::value, pqxx::result>::type
PostgreSQLConnection::exec_params_p(
    const std::string& theSQLStatement,
    const Container& container,
    int requested_size) const
{
  // No easy way to use the same method name as for Args... as we need to distinguish from
  // cases when std::string<something> is provided as the only argument
  try
  {
    pqxx::params params;
    params.append_multi(container);
    pqxx::result result = exec_params(theSQLStatement, params);
    if (requested_size >= 0 && requested_size != static_cast<int>(result.size()))
    {
      throw Fmi::Exception(BCP,
          fmt::format("Expected {} rows but got {} rows", requested_size, result.size()));
    }
    return result;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

using PostgreSQLConnectionPool = Fmi::Pool
<
  Fmi::PoolInitType::Parallel,
  PostgreSQLConnection,
  PostgreSQLConnectionOptions
>;

}  // namespace Database
}  // namespace Fmi

// ======================================================================
