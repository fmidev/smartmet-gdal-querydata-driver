// ======================================================================
/*!
 * \file
 * \brief Regression tests for Streams.h
 */
// ======================================================================

#include "FileSystem.h"
#include "Exception.h"
#include <regression/tframe.h>
#include <sstream>
#include <fstream>
#include <filesystem>
#include <vector>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/device/file_descriptor.hpp>
#include <stdio.h>

namespace fs = std::filesystem;
namespace io = boost::iostreams;

std::string read_orig_file()
{
    std::string result;
    std::ifstream test_input("FileSystemTest.cpp");
    io::copy(test_input, std::back_inserter(result));
    return result;
}

void read_gzip_compressed_stream()
{
    std::string s1, s2 = read_orig_file();

    std::filesystem::create_directories("tmp");
    system("cat FileSystemTest.cpp | gzip >tmp/FileSystemTest.cpp.gz");
    std::ifstream pipe("tmp/FileSystemTest.cpp.gz");
    Fmi::IStream input(pipe, "tmp/FileSystemTest.cpp.gz");
    io::copy(input, std::back_inserter(s1));

    if (s1 != s2)
        TEST_FAILED("Failed to decompress GZIP compressed file");

    TEST_PASSED();
}

void read_xz_compressed_stream()
{
    std::string s1, s2 = read_orig_file();

    std::filesystem::create_directory("tmp");
    system("cat FileSystemTest.cpp | xz >tmp/FileSystemTest.cpp.xz");
    std::ifstream pipe("tmp/FileSystemTest.cpp.xz");
    Fmi::IStream input(pipe, Fmi::Compression::XZ);
    io::copy(input, std::back_inserter(s1));

    if (s1 != s2)
    {
        TEST_FAILED("Failed to decompress XZ compressed file");
    }
    TEST_PASSED();
}

std::string compress(const std::string& src, Fmi::Compression compression)
{
    std::istringstream input(src);
    std::ostringstream raw_output;
    Fmi::OStream output(raw_output, compression);
    io::copy(input, output);
    return raw_output.str();
}

std::string decompress(const std::string& src, Fmi::Compression compression)
{
    std::istringstream raw_input(src);
    std::ostringstream output;
    Fmi::IStream input(raw_input, compression);
    io::copy(input, output);
    return output.str();
}

void test_xz()
{
    std::string s0 = read_orig_file();
    std::string s1 = compress(s0, Fmi::Compression::XZ);
    std::string s2 = decompress(s1, Fmi::Compression::XZ);

    if (s0 != s2)
        TEST_FAILED("XZ compression/decompression test failed");

    TEST_PASSED();
}

void test_zstd()
{
    std::string s0 = read_orig_file();
    std::string s1 = compress(s0, Fmi::Compression::ZSTD);
    std::string s2 = decompress(s1, Fmi::Compression::ZSTD);

    if (s0 != s2)
        TEST_FAILED("ZSTD compression/decompression test failed");

    TEST_PASSED();
}

void test_bzip2()
{
    std::string s0 = read_orig_file();
    std::string s1 = compress(s0, Fmi::Compression::BZIP2);
    std::string s2 = decompress(s1, Fmi::Compression::BZIP2);

    if (s0 != s2)
        TEST_FAILED("BZIP2 compression/decompression test failed");

    TEST_PASSED();
}

void test_gzip()
{
    std::string s0 = read_orig_file();
    std::string s1 = compress(s0, Fmi::Compression::GZIP);
    std::string s2 = decompress(s1, Fmi::Compression::GZIP);

    if (s0 != s2)
        TEST_FAILED("ZSTD compression/decompression test failed");

    TEST_PASSED();
}

void test_no_compression()
{
    std::string s0 = read_orig_file();
    std::string s1 = compress(s0, Fmi::Compression::NONE);
    std::string s2 = decompress(s1, Fmi::Compression::NONE);

    if (s0 != s1)
        TEST_FAILED("No compression/decompression test failed");

    if (s0 != s2)
        TEST_FAILED("No compression/decompression test failed");

    TEST_PASSED();
}

void test_last_write_time()
{
    std::error_code ec;
    const std::time_t t = std::time(nullptr);
    std::filesystem::create_directories("tmp");
    std::ofstream out("tmp/test.txt");
    out << "test";
    out.close();
    const std::time_t t2 = Fmi::last_write_time("tmp/test.txt", ec);
    if (ec)
    {
        TEST_FAILED("Failed to get last write time. Error: " + ec.message());
    }

    if (std::abs(t2 - t) > 2)
    {
        std::cout << "now=" << t << " last=" << t2 << std::endl;
        TEST_FAILED("Last write time differs too much");
    }

    std::time_t t2a = Fmi::last_write_time_or("tmp/test.txt", 1);
    if (t2 != t2a)
    {
        TEST_FAILED("Fmi::last_write_time_or() must return the same resylt as Fmi::last_write_time if file exists");
    }

    Fmi::last_write_time("tmp/nonexistent.txt", ec);
    if (!ec)
    {
        TEST_FAILED("last_write_time should have failed");
    }
    if (ec.value() != ENOENT)
    {
        TEST_FAILED("last_write_time failed with wrong error code: " + ec.message());
    }
    t2a = Fmi::last_write_time_or("tmp/nonexistent.txt", 1);
    if (t2a != 1)
    {
        TEST_FAILED("Fmi::last_write_time_or() must return provided default value in case of an error getting last write time ("
                "got " + std::to_string(t2a) + ")");
    }

    try
    {
        (void)Fmi::last_write_time("tmp/nonexistent.txt");
        TEST_FAILED("last_write_time should have thrown");
    }
    catch(const Fmi::Exception&)
    {
    }
    catch(...)
    {
        TEST_FAILED("last_write_time should have thrown Fmi::Exception");
    }

    TEST_PASSED();
}

void test_unique_path()
{
    int num_err = 0;
    std::vector<fs::path> paths;
    fs::create_directories("tmp");
    for (int i = 0; i < 100; i++)
    {
        const fs::path p = Fmi::unique_path("tmp/%%%%-%%%%-%%%%-%%%%");
        if (fs::exists(p))
        {
            ++num_err;
        }
        else
        {
            std::ofstream ofs(p.c_str());
            if (!ofs)
            {
                ++num_err;
            }
            else
            {
                ofs << "test";
                ofs.close();
            }
        }
        paths.push_back(Fmi::unique_path("tmp/%%%%-%%%%-%%%%-%%%%"));
    }

    // Make sure all paths are unique
    const std::size_t n = paths.size();
    std::sort(paths.begin(), paths.end());
    paths.erase(std::unique(paths.begin(), paths.end()), paths.end());

    if (num_err)
    {
        TEST_FAILED(std::to_string(num_err) + " errors while creating unique paths");
    }

    if (paths.size() != n)
    {
        TEST_FAILED("Not all unique paths are unique");
    }

    TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * The actual test suite
 */
// ----------------------------------------------------------------------

class tests : public tframe::tests
{
  virtual const char* error_message_prefix() const { return "\n\t"; }

  void test(void)
  {
    TEST(read_gzip_compressed_stream);
    TEST(read_xz_compressed_stream);
    TEST(test_xz);
    TEST(test_zstd);
    TEST(test_bzip2);
    TEST(test_gzip);
    TEST(test_no_compression);
    TEST(test_last_write_time);
    TEST(test_unique_path);

    fs::remove_all("tmp");
  }
};

int main(void)
{
  std::cout << std::endl << "FileSystem utils tester" << std::endl << "=====================" << std::endl;
  tests t;

  return t.run();
}
