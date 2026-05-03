#include "MappedFile.h"
#include <boost/test/included/unit_test.hpp>
#include <cmath>
#include <cstring>
#include <fstream>
#include <unistd.h>
#include "Exception.h"

using namespace boost::unit_test;

namespace
{
    struct Cleanup
    {
        Cleanup(const std::string& fn) : fn(fn) {}
        ~Cleanup() { unlink(fn.c_str()); }
        const std::string fn;
    };
}

test_suite* init_unit_test_suite(int argc, char* argv[])
{
  const char* name = "MappedFile tester";
  unit_test_log.set_threshold_level(log_messages);
  framework::master_test_suite().p_name.value = name;
  BOOST_TEST_MESSAGE("");
  BOOST_TEST_MESSAGE(name);
  BOOST_TEST_MESSAGE(std::string(std::strlen(name), '='));
  return nullptr;
}

BOOST_AUTO_TEST_CASE(test_mapping_file_ro)
{
    try {
        Fmi::MappedFile mapped_file;
        mapped_file.open("MappedFileTest.cpp");
        const char* data = mapped_file.const_data();
        BOOST_REQUIRE(data != nullptr);
        const std::size_t size = mapped_file.size();
        std::string content(data, data + size);
        BOOST_REQUIRE(content.find("MappedFileTest.cpp") != std::string::npos);
    } catch (Fmi::Exception& e) {
        std::cout << e << std::endl;
        BOOST_REQUIRE(false);
    }
}

BOOST_AUTO_TEST_CASE(test_mapping_file_rw)
{
    try {
        constexpr std::size_t size = 1024;

        char buffer[size];
        for (std::size_t i = 0; i < size; i++)
        {
            buffer[i] = i % 256U;
        }

        const std::string fn = "MappedFile.tmp";
        Cleanup cleanup(fn);
        Fmi::MappedFile::Params params(fn);
        params.flags = Fmi::MappedFile::mapmode::readwrite;
        params.new_file_size = size;

        Fmi::MappedFile mapped_file;
        mapped_file.open(params);
        char* data = mapped_file.data();
        BOOST_REQUIRE(data != nullptr);
        std::memcpy(buffer, data, size);
        mapped_file.close();

        std::ostringstream tmp;
        std::ifstream input(fn);
        std::copy(
            std::istreambuf_iterator<char>(input.rdbuf()),
            std::istreambuf_iterator<char>(),
            std::ostream_iterator<char>(tmp));
        BOOST_CHECK_EQUAL(tmp.str().length(), size);
        BOOST_REQUIRE(memcmp(buffer, tmp.str().c_str(), size) == 0);
    } catch (Fmi::Exception& e) {
        std::cout << e << std::endl;
        BOOST_REQUIRE(false);
    }
}

BOOST_AUTO_TEST_CASE(test_mapping_file_with_offset)
{
    try {
        // Create a temporary file with known content
        const std::string fn = "MappedFileOffset.tmp";
        Cleanup cleanup(fn);
        
        // Use page-aligned offset (boost::iostreams requires this)
        // but our madvise logic should handle the actual memory mapping correctly
        const long page_size = sysconf(_SC_PAGESIZE);
        constexpr std::size_t total_size = 16384;
        const std::size_t offset = page_size;  // Page-aligned offset
        constexpr std::size_t mapped_size = 4000;
        
        // Write test data to file
        {
            std::ofstream out(fn, std::ios::binary);
            for (std::size_t i = 0; i < total_size; i++)
            {
                out.put(static_cast<char>(i % 256));
            }
            out.close();  // Explicitly close the file
        }
        
        // Map file starting from offset using open() method
        Fmi::MappedFile mapped_file;
        mapped_file.open(fn, 
                         Fmi::MappedFile::mapmode::readonly,
                         mapped_size,
                         offset);
        
        BOOST_REQUIRE(mapped_file.is_open());
        const char* data = mapped_file.const_data();
        BOOST_REQUIRE(data != nullptr);
        BOOST_CHECK_EQUAL(mapped_file.size(), mapped_size);
        
        // Verify the content matches what should be at the offset
        for (std::size_t i = 0; i < mapped_size; i++)
        {
            std::size_t expected_value = (offset + i) % 256;
            unsigned char actual_value = static_cast<unsigned char>(data[i]);
            if (actual_value != expected_value)
            {
                BOOST_ERROR("Mismatch at position " << i 
                           << ": expected " << expected_value 
                           << ", got " << static_cast<int>(actual_value));
                break;
            }
        }
        
        // Test that close works correctly with offset
        mapped_file.close();
        BOOST_CHECK(!mapped_file.is_open());
        
        // Test reopening with different offset using Params
        Fmi::MappedFile::Params params(fn);
        params.flags = Fmi::MappedFile::mapmode::readonly;
        params.length = 3000;
        params.offset = page_size * 2;  // Different page-aligned offset
        
        mapped_file.open(params);
        BOOST_REQUIRE(mapped_file.is_open());
        data = mapped_file.const_data();
        BOOST_REQUIRE(data != nullptr);
        BOOST_CHECK_EQUAL(mapped_file.size(), 3000);
        
        // Verify content at new offset
        for (std::size_t i = 0; i < 3000; i++)
        {
            std::size_t expected_value = (page_size * 2 + i) % 256;
            unsigned char actual_value = static_cast<unsigned char>(data[i]);
            if (actual_value != expected_value)
            {
                BOOST_ERROR("Mismatch at position " << i 
                           << " with new offset: expected " << expected_value 
                           << ", got " << static_cast<int>(actual_value));
                break;
            }
        }
        
    } catch (Fmi::Exception& e) {
        std::cout << e << std::endl;
        BOOST_REQUIRE(false);
    }
}
