#include <iostream>
#include <thread>
#include <vector>
#include "Exception.h"
#include "PostgreSQLConnection.h"
#include "Pool.h"

class TortureTest
{
    std::atomic<std::size_t> success{0};
    std::atomic<std::size_t> errors{0};
    Fmi::Database::PostgreSQLConnectionOptions connOptions;
    Fmi::Database::PostgreSQLConnectionPool connPool;

public:
    TortureTest()
        : connOptions("host=127.0.0.1 port=15432 dbname=fminames user=fminames_user password=fminames_pw")
        , connPool(5, 20, connOptions)
    {
    }

    void run()
    {
        const int num_threads = 50;
        std::thread status_thread(&TortureTest::report_status_thread, this);
        std::vector<std::thread> threads;
        for (int i = 0; i < num_threads; ++i)
        {
            threads.emplace_back(&TortureTest::test_thread_proc, this);
        }

        for (auto& thread : threads)
        {
            thread.join();
        }
    }

private:

    void report_status_thread()
    {
        while (true)
        {
            std::this_thread::sleep_for(std::chrono::seconds(5));
            std::cout << "Status: " << success.load() << " successful tests, "
                      << errors.load() << " errors." << std::endl;
        }
    }

    void test_thread_proc()
    {
        while (true)
        {
            try
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                single_test();
            }
            catch (const Fmi::Exception& ex)
            {
                std::cerr << "Torture test thread caught exception: " << ex << std::endl;
            }
        }
    }

    void single_test()
    try
    {
        Fmi::Database::PostgreSQLConnectionPool::Ptr connPtr = connPool.get();
            pqxx::result result =
        connPtr->exec_params(
                "SELECT id, lat, lon FROM geonames WHERE name=$1",
                "Valassaaret");
        if (result.size() != 1)
        {
            throw Fmi::Exception(BCP, "Unexpected result size");
        }
        int id = result[0]["id"].as<int>();
        if (id != 632561)
        {
            throw Fmi::Exception(BCP, "Unexpected id value");
        }
        ++success;
    }
    catch (...)
    {
        ++errors;
        throw Fmi::Exception::Trace(BCP, "Operation failed!");
    }
};

int main(int argc, char** argv)
try
{
    TortureTest test;
    test.run();
    return 0;
}
catch (...)
{
    std::cerr << "Torture test main caught exception: " << Fmi::Exception::Trace(BCP, "Operation failed") << std::endl;
    return 1;
}
