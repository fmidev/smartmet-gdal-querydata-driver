#include "PostgreSQLConnectionImpl.h"
#include "Exception.h"
#include <boost/chrono.hpp>
#include <boost/thread.hpp>
#include <fmt/format.h>
#include <memory>
#include <utility>

using Fmi::Database::PostgreSQLConnection;

static_assert(PQXX_VERSION_MAJOR >= 7, "pqxx version 7 or higher is required");

namespace
{
    pqxx::result transaction_exec_params(
        pqxx::transaction_base& transaction,
        const std::string& sql,
        pqxx::params params)
    {
        #if PQXX_VERSION_MAJOR >= 8 || PQXX_VERSION_MINOR >= 10
            return transaction.exec(sql, params);
        #else
            return transaction.exec_params(sql, params);
        #endif
    }

    pqxx::result transaction_exec_prepared(
        pqxx::transaction_base& transaction,
        const std::string& name,
        pqxx::params params)
    {
        #if PQXX_VERSION_MAJOR >= 8 || PQXX_VERSION_MINOR >= 10
            return transaction.exec(pqxx::prepped(name), params);
        #else
            return transaction.exec_prepared(name, params);
        #endif
    }
}  // namespace

PostgreSQLConnection::Impl::Impl(const PostgreSQLConnectionOptions& theConnectionOptions)
    : itsDebug(theConnectionOptions.debug),
      itsCanceled(false),
      itsConnectionOptions(theConnectionOptions)
{
    open(itsConnectionOptions);
}

bool PostgreSQLConnection::Impl::open(const PostgreSQLConnectionOptions& theConnectionOptions)
try
{
    itsConnectionOptions = theConnectionOptions;
    std::size_t numReconnectAttempts =
        reconnectDisabled.load()
            ? 0
            : (last_failed ? 9 : 2); // If last connection attempt failed, try less times
    itsConnection = open_internal(numReconnectAttempts);
    return bool(itsConnection);
}
catch (const std::exception& e)
{
    throw Fmi::Exception::Trace(BCP, "Error opening PostgreSQL connection to "
        + fmt::format("{}", getId()) + ": " + e.what());
    return false;
}

bool PostgreSQLConnection::Impl::reopen()
try
{
    std::size_t numReconnectAttempts =
        reconnectDisabled.load()
            ? 0
            : (last_failed ? 9 : 2); // If last connection attempt failed, try less times
    itsConnection = open_internal(numReconnectAttempts);
    return bool(itsConnection);
}
catch (const std::exception& e)
{
    throw Fmi::Exception::Trace(BCP, "Error reopening PostgreSQL connection to "
        + fmt::format("{}", getId()) + ": " + e.what());
    return false;
}


void PostgreSQLConnection::Impl::close()
{
  itsTransaction.reset();
  itsConnection.reset();
}


void PostgreSQLConnection::Impl::startTransaction()
try
{
    itsTransaction = retry_on_failure(queryRetryLimit, &Impl::try_create_transaction);
}
catch (const pqxx::failure& e)
{
    throw Fmi::Exception::Trace(BCP, "Error starting PostgreSQL transaction to "
        + fmt::format("{}", getId()) + ": " + e.what());
}

void PostgreSQLConnection::Impl::endTransaction()
try
{
    if (itsTransaction)
      itsTransaction.reset();
    else
      throw Fmi::Exception(BCP, "Not in transaction");
}
catch (const pqxx::failure& e)
{
    throw Fmi::Exception::Trace(BCP, "Error ending PostgreSQL transaction to "
        + fmt::format("{}", getId()) + ": " + e.what());
}



void PostgreSQLConnection::Impl::commitTransaction()
try
{
    if (itsTransaction)
      itsTransaction->commit();
    else
      throw Fmi::Exception(BCP, "Not in transaction");
}
catch (const pqxx::failure& e)
{
    throw Fmi::Exception::Trace(BCP, "Error committing PostgreSQL transaction to "
        + fmt::format("{}", getId()) + ": " + e.what());
}



void PostgreSQLConnection::Impl::exec(bool require_transaction, const std::string& sql)
{
    /* Discard result */ execute(require_transaction, sql, pqxx::params{});
}


pqxx::result PostgreSQLConnection::Impl::execute(bool require_transaction, const std::string& sql, pqxx::params params)
try
{
    if (itsTransaction)
    {
        return retry_on_failure<pqxx::result, bool, const std::string&, pqxx::params>(
            PostgreSQLConnection::queryRetryLimit, &Impl::try_execute, require_transaction, sql, params);
    }

    if (require_transaction) throw Fmi::Exception(BCP, "Not in transaction");

    return retry_on_failure<pqxx::result, const std::string&, pqxx::params>(1, &Impl::try_execute_non_transaction, sql, params);
}
catch (const pqxx::failure& e)
{
    throw Fmi::Exception::Trace(BCP, "Error executing SQL in PostgreSQL transaction to "
        + fmt::format("{}", getId()) + ": " + e.what());
}


pqxx::result PostgreSQLConnection::Impl::executeNonTransaction(const std::string& sql, pqxx::params params)
try
{
    return retry_on_failure<pqxx::result, const std::string&, pqxx::params>(
        queryRetryLimit,
        &Impl::try_execute_non_transaction,
        sql,
        params);
}
catch (const pqxx::failure& e)
{
    throw Fmi::Exception::Trace(BCP, "Error executing SQL in PostgreSQL non-transaction to "
        + fmt::format("{}", getId()) + ": " + e.what());
}


std::string PostgreSQLConnection::Impl::quote(const std::string& theString)
try
{
    return retry_on_failure<std::string, const std::string&>(queryRetryLimit, &Impl::try_quote, theString);
}
catch (const pqxx::failure& e)
{
    throw Fmi::Exception::Trace(BCP, "Error quoting string for PostgreSQL connection to "
        + fmt::format("{}", getId()) + ": " + e.what());
}


void PostgreSQLConnection::Impl::cancel()
{
    if (itsConnection)
    {
      itsCanceled.store(true);
      itsConnection->cancel_query();
      boost::unique_lock<boost::mutex> lock(m);
      cond.notify_one();
    }
}


bool PostgreSQLConnection::Impl::isConnected() const
{
    if (itsConnection)
    {
        return itsConnection->is_open();
    }
    return false;
}


std::shared_ptr<pqxx::transaction_base> PostgreSQLConnection::Impl::get_transaction_impl()
{
    return retry_on_failure<std::shared_ptr<pqxx::transaction_base>>(
        queryRetryLimit, &Impl::try_get_transaction_impl);
}


void PostgreSQLConnection::Impl::setClientEncoding(const std::string& theEncoding)
try
{
    retry_on_failure<void, const std::string&>(queryRetryLimit, &Impl::try_set_client_encoding, theEncoding);
    itsEncoding = theEncoding;
}
catch (const pqxx::failure& e)
{
    itsEncoding = theEncoding;
    throw Fmi::Exception::Trace(BCP, "Error setting client encoding for PostgreSQL connection to "
        + fmt::format("{}", getId()) + ": " + e.what());
}


void PostgreSQLConnection::Impl::register_prepared_statement(const std::string& name, const std::string& sql)
try
{
    retry_on_failure<void, const std::string&, const std::string&>(queryRetryLimit, &Impl::try_register_prepared_statement, name, sql);
}
catch (const pqxx::failure& e)
{
    throw Fmi::Exception::Trace(BCP, "Error registering prepared SQL statement '" + name
        + "' for PostgreSQL connection to " + fmt::format("{}", getId()) + ": " + e.what());
}


void PostgreSQLConnection::Impl::unregister_prepared_statement(const std::string& name)
try
{
    // No need to retry on failure here. Just remove from local map and from server if connected.
    prepared_sqls.erase(name);
    if (itsConnection)
        itsConnection->unprepare(name);
}
catch (const pqxx::failure& e)
{
    throw Fmi::Exception::Trace(BCP, "Error unregistering prepared SQL statement '" + name
        + "' for PostgreSQL connection to " + fmt::format("{}", getId()) + ": " + e.what());
}

std::string PostgreSQLConnection::Impl::get_prepared_sql_string(const std::string& name) const
{
    auto it = prepared_sqls.find(name);
    if (it != prepared_sqls.end())
        return it->second;

    return "Unknown prepared SQL statement '" + name + "' for PostgreSQL connection to "
        + fmt::format("{}", getId());
}



std::shared_ptr<pqxx::connection> PostgreSQLConnection::Impl::open_internal(std::size_t numReconnectAttempts)
{
    itsCanceled.store(false);
    last_failed = false;

    std::shared_ptr<pqxx::connection> connection;

    std::string conn_str = static_cast<std::string>(itsConnectionOptions);

    std::exception_ptr last_exception;

    for (std::size_t attempt = 0; attempt <= numReconnectAttempts; ++attempt)
    {
        boost::this_thread::interruption_point();
        try
        {
            if (itsConnection)
                close();

            connection = std::make_shared<pqxx::connection>(conn_str);
            finalize_connection(*connection); // Check server capabilities and prepare statements earlier registered
            last_failed = false;
            return connection;
        }
        catch (const pqxx::broken_connection& e)
        {
            last_exception = std::current_exception();
            last_failed = true;
        }

        if (attempt < numReconnectAttempts)
        {
            int delay = 10 + 3 * attempt / 3;
            boost::this_thread::sleep_for(boost::chrono::seconds(delay));
        }
        else if (last_exception)
        {
            std::cout << "Failed to open PostgreSQL connection to" + fmt::format(" {}", getId())
                         + " after " + std::to_string(numReconnectAttempts) + " attempts: "
                      << Fmi::Exception::Trace(BCP, "Connection error").getWhat() << std::endl;
            last_failed = true;
            std::rethrow_exception(last_exception);
        }
    }

    Fmi::Exception error(BCP, "INTERNAL ERROR: no supposed to be here (connecting to " + fmt::format("{}", getId()) + ")");
    throw error;
}


void PostgreSQLConnection::Impl::finalize_connection(pqxx::connection& connection)
{
    pqxx::result result_set;
    const std::string sql = "select typname,oid from pg_type";
    try
    {
        // Do not use executeNonTransaction here, as we do not want to call reopen() recursively
        pqxx::nontransaction nitsTransaction(connection);
        result_set = nitsTransaction.exec(sql);
    }
    catch (const pqxx::broken_connection& e)
    {
        // Add message and rethrow
        std::cout << "Failed to execute '" << sql << "' on connection " << fmt::format("{}", getId())
                  << ": " << e.what() << std::endl;
        throw;
    }

    for (auto row : result_set)
    {
        const auto datatype = row[0].as<std::string>();
        const auto oid = row[1].as<unsigned int>();
        itsDataTypes.insert(std::make_pair(oid, datatype));
    }

    for (const auto& item : prepared_sqls)
    {
        boost::this_thread::interruption_point();
        try
        {
            connection.prepare(item.first, item.second);
        }
        catch (const pqxx::broken_connection& e)
        {
            // Add message and rethrow
            std::cout << "Failed to prepare SQL statement '" << item.first << "' on connection " << fmt::format("{}", getId())
                      << ": " << e.what() << std::endl;
            throw;
        }
    }

    if (itsEncoding)
    {
        // Encoding was set earlier before reconnect. Set it again.
        connection.set_client_encoding(itsEncoding->c_str());
    }
}


void PostgreSQLConnection::Impl::checkSlowQuery(
    const std::string& theSQLStatement,
    const std::chrono::time_point<std::chrono::high_resolution_clock>& start,
    const std::chrono::time_point<std::chrono::high_resolution_clock>& end) const
{
    auto limit = itsConnectionOptions.slow_query_limit;

    if (limit > 0)
    {
        const auto duration = std::chrono::duration_cast<std::chrono::seconds>(end - start);
        if (duration.count() >= limit)
        {
            auto sql = theSQLStatement;
            std::replace(sql.begin(), sql.end(), '\n', ' ');
            std::cerr << fmt::format("{} Slow {}:{} query took {} seconds, limit is {}. SQL: {}\n",
                                 Fmi::to_simple_string(Fmi::SecondClock::local_time()),
                                 itsConnectionOptions.host,
                                 itsConnectionOptions.database,
                                 duration.count(),
                                 limit,
                                 sql);
        }
    }
}



std::shared_ptr<pqxx::work> PostgreSQLConnection::Impl::try_create_transaction()
{
    return std::make_shared<pqxx::work>(*itsConnection);
}


pqxx::result PostgreSQLConnection::Impl::try_execute(
    bool require_transaction,
    const std::string& sql,
    pqxx::params params)
{
    if (itsTransaction)
    {
        const auto start = std::chrono::high_resolution_clock::now();
        pqxx::result result = transaction_exec_params(*itsTransaction, sql, params);
        const auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> duration = end - start;
        checkSlowQuery(sql, start, end);
        return result;
    }

    if (require_transaction) throw Fmi::Exception(BCP, "Not in transaction");

    pqxx::nontransaction nitsTransaction(*itsConnection);
    const auto start = std::chrono::high_resolution_clock::now();
    pqxx::result result = transaction_exec_params(nitsTransaction, sql, params);
    const auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration = end - start;
    checkSlowQuery(sql, start, end);
    return result;
}


pqxx::result PostgreSQLConnection::Impl::try_execute_non_transaction(const std::string& sql, pqxx::params params)
{
    pqxx::nontransaction nitsTransaction(*itsConnection);
    const auto start = std::chrono::high_resolution_clock::now();
    pqxx::result result = transaction_exec_params(nitsTransaction, sql, params);
    const auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration = end - start;
    checkSlowQuery(sql, start, end);
    return result;
}



pqxx::result PostgreSQLConnection::Impl::exec_prepared(
    bool require_transaction,
    const std::string& name,
    pqxx::params params)
try
{
    return retry_on_failure<pqxx::result, bool, const std::string&, pqxx::params>(
        1, &Impl::try_exec_prepared, require_transaction, name, params);
}
catch (const pqxx::failure& e)
{
    throw Fmi::Exception::Trace(BCP, "Error executing prepared SQL statement '" + name
        + "' for PostgreSQL connection to " + fmt::format("{}", getId()) + ": " + e.what());
}



pqxx::result PostgreSQLConnection::Impl::exec_params(
    bool require_transaction,
    const std::string& sql,
    pqxx::params params)
try
{
    if (itsTransaction)
    {
        return retry_on_failure<pqxx::result, bool, const std::string&, pqxx::params>(
            1, &Impl::try_execute, require_transaction, sql, params);
    }

    if (require_transaction) throw Fmi::Exception(BCP, "Not in transaction");

    return retry_on_failure<pqxx::result, const std::string&, pqxx::params>(
        1, &Impl::try_execute_non_transaction, sql, params);
}
catch (const pqxx::failure& e)
{
    throw Fmi::Exception::Trace(BCP, "Error executing SQL statement for PostgreSQL connection to "
        + fmt::format("{}", getId()) + ": " + e.what());
}




pqxx::result PostgreSQLConnection::Impl::try_exec_prepared(
    bool require_transaction,
    const std::string& name,
    pqxx::params params)
{
    const std::string prepared_sql = get_prepared_sql_string(name);
    if (itsTransaction)
    {
        const auto start = std::chrono::high_resolution_clock::now();
        const pqxx::result result = transaction_exec_prepared(*itsTransaction, name, params);
        const auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> duration = end - start;
        checkSlowQuery(prepared_sql, start, end);
        return result;
    }

    if (require_transaction) throw Fmi::Exception(BCP, "Not in transaction");

    const auto start = std::chrono::high_resolution_clock::now();
    pqxx::nontransaction nitsTransaction(*itsConnection);
    const pqxx::result result = transaction_exec_prepared(nitsTransaction, name, params);
    const auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration = end - start;
    checkSlowQuery(prepared_sql, start, end);
    return result;
}


std::shared_ptr<pqxx::transaction_base> PostgreSQLConnection::Impl::try_get_transaction_impl()
{
    if (itsTransaction)
        return itsTransaction;
    return std::make_shared<pqxx::nontransaction>(*itsConnection);
}


std::string PostgreSQLConnection::Impl::try_quote(const std::string& str)
{
    return itsConnection->quote(str);
}


void PostgreSQLConnection::Impl::try_set_client_encoding(const std::string& theEncoding)
{
    itsConnection->set_client_encoding(theEncoding.c_str());
}


void PostgreSQLConnection::Impl::try_register_prepared_statement(const std::string& name, const std::string& sql)
{
    itsConnection->prepare(name, sql);
    prepared_sqls[name] = sql;
}


void PostgreSQLConnection::Impl::try_unregister_prepared_statement(const std::string& name)
{
    prepared_sqls.erase(name);
    itsConnection->unprepare(name);
}
