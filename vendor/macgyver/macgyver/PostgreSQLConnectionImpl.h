// This header file is not intended to be self sufficient and requires inclusion
// of PostgreSQLConnection.h beforehand. It is also not intended to be included
// directly by user code.
#pragma once

#include "PostgreSQLConnection.h"
#include <algorithm>
#include <iostream>
#include <memory>
#include <map>
#include <optional>
#include <boost/thread.hpp>
#include <fmt/format.h>

namespace Fmi
{
namespace Database
{
    class PostgreSQLConnection::Impl final
    {
        mutable std::shared_ptr<pqxx::connection> itsConnection;  // PostgreSQL connecton
        mutable std::shared_ptr<pqxx::work> itsTransaction;       // PostgreSQL transaction
        bool itsDebug = false;
        bool itsCollate = false;
        std::atomic<bool> itsCanceled;
        PostgreSQLConnectionOptions itsConnectionOptions;
        mutable std::map<unsigned int, std::string> itsDataTypes;
        mutable boost::condition_variable cond;
        mutable boost::mutex m;

        mutable bool last_failed = false;

        std::map<std::string, std::string> prepared_sqls;
        std::optional<std::string> itsEncoding;

    public:
        Impl(bool debug) : itsDebug(debug) {}

        Impl(const PostgreSQLConnectionOptions& theConnectionOptions);

        ~Impl() { close(); }

        auto getId() const -> PostgreSQLConnectionId
        {
            return itsConnectionOptions.getId();
        }

        bool open(const PostgreSQLConnectionOptions& theConnectionOptions);

        bool reopen();

        void close();

        void startTransaction();

        void endTransaction();

        void commitTransaction();

        void exec(bool require_transaction, const std::string& sql);

        pqxx::result execute(bool require_transaction, const std::string& sql, pqxx::params params);

        pqxx::result executeNonTransaction(const std::string& sql, pqxx::params params);

        pqxx::result exec_prepared(bool require_transaction, const std::string& name, pqxx::params params);

        pqxx::result exec_params(bool require_transaction, const std::string& sql, pqxx::params params);


        std::string quote(const std::string& theString);

        void cancel();

        bool isConnected() const;

        bool isCanceled() const { return itsCanceled.load(); }

        bool collateSupported() const { return itsCollate; }

        const std::map<unsigned int, std::string>& dataTypes() const { return itsDataTypes; }

        bool isDebug() const { return itsDebug; }

        void setDebug(bool debug) { itsDebug = debug; }

        std::shared_ptr<pqxx::connection> get_connection() const
        {
            if (itsConnection && isConnected())
                return itsConnection;

            return {};
        }

        bool isTransaction() const { return !!itsTransaction; }

        std::shared_ptr<pqxx::transaction_base> get_transaction_impl();

        void setClientEncoding(const std::string& theEncoding);

        void register_prepared_statement(const std::string& name, const std::string& sql);

        void unregister_prepared_statement(const std::string& name);

    private:
        Impl(const Impl&) = delete;
        Impl(Impl&&) = delete;
        Impl& operator=(const Impl&) = delete;
        Impl& operator=(Impl&&) = delete;

        template <typename ReturnType, typename... Args>
        using MethodType = ReturnType (PostgreSQLConnection::Impl::*)(Args...);

        template <typename ReturnType, typename... Args>
        ReturnType retry_on_failure(std::size_t numTries, MethodType<ReturnType, Args...> method, Args... args);

        std::shared_ptr<pqxx::connection> open_internal(std::size_t numReconnectAttempts);

        void finalize_connection(pqxx::connection& connection);

        void checkSlowQuery(const std::string& theSQLStatement,
                            const std::chrono::time_point<std::chrono::high_resolution_clock>& start,
                            const std::chrono::time_point<std::chrono::high_resolution_clock>& end) const;

        std::shared_ptr<pqxx::work> try_create_transaction();

        pqxx::result try_execute(bool require_transaction, const std::string& sql, pqxx::params params);

        pqxx::result try_execute_non_transaction(const std::string& sql, pqxx::params params);

        std::string try_quote(const std::string& theString);

        void try_set_client_encoding(const std::string& theEncoding);

        void try_register_prepared_statement(const std::string& name, const std::string& sql);

        void try_unregister_prepared_statement(const std::string& name);

        std::string get_prepared_sql_string(const std::string& name) const;

        pqxx::result try_exec_prepared(bool require_transaction, const std::string& name, pqxx::params params);

        std::shared_ptr<pqxx::transaction_base> try_get_transaction_impl();
    };


    template <typename ReturnType, typename... Args>
    ReturnType PostgreSQLConnection::Impl::retry_on_failure(
        std::size_t numTries,
        MethodType<ReturnType, Args...> method,
        Args... args)
    {
        // Override the number of retries if reconnect is disabled
        if (PostgreSQLConnection::reconnectDisabled.load())
        {
            numTries = 0;
        }

        last_failed = false;
        bool has_connection = bool(itsConnection);
        std::optional<boost::chrono::seconds> wait_duration;

        for (std::size_t attempt = 0; attempt <= numTries; ++attempt)
        {
            std::exception_ptr last_exception;
            try
            {
                if (wait_duration)
                {
                    boost::unique_lock<boost::mutex> lock(m);
                    cond.wait_for(lock, *wait_duration);
                    wait_duration.reset();
                }

                if (!has_connection)
                    itsConnection = open_internal(0);

                boost::this_thread::interruption_point();

                return (this->*method)(args...);
            }
            catch(const pqxx::broken_connection& e)
            {
                last_failed = true;
                const int remaining = numTries - attempt;
                if (remaining > 0)
                {
                    std::cout << "PostgreSQL connection to " + fmt::format("{}", getId())
                                 + " broken during operation : " + std::string(e.what())
                              << " (attempts remaining " << numTries - attempt << ")"
                              << std::endl;
                    itsConnection.reset();
                    wait_duration = boost::chrono::seconds(10 + 3 * (attempt / 3));
                    has_connection = false;
                }
                else
                {
                    std::cout << "PostgreSQL connection to " + fmt::format("{}", getId())
                                 + " broken during operation and no more retries left: "
                                 + std::string(e.what())
                              << std::endl;
                    last_failed = true;
                    throw;
                }
            }
        }

        throw Fmi::Exception(BCP, "INTERNAL ERROR: no supposed to be here (PostgreSQLConnectionImpl::retry_on_failure)");
    }

} // namespace Database
} // namespace Fmi
