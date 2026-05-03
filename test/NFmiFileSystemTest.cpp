// ======================================================================
/*!
 * \file
 * \brief Regression tests for class NFmiFileSystem
 */
// ======================================================================

#include "NFmiFileSystem.h"
#include "NFmiStringTools.h"
#include <regression/tframe.h>
#include <macgyver/FileSystem.h>
#include <macgyver/StringConversion.h>
#include <list>
#include <string>

extern "C"
{
#include <unistd.h>  // sleep
}

//! Protection against conflicts with global functions
namespace NFmiFileSystemTest
{
// ----------------------------------------------------------------------
/*!
 * Tests the FileComplete function
 */
// ----------------------------------------------------------------------

void filecomplete(void)
{
  using namespace std;
  using NFmiFileSystem::FileComplete;

  std::string result;

  result = FileComplete("NFmiFileSystemTest.cpp", "");
  if (result != "NFmiFileSystemTest.cpp")
    TEST_FAILED("Should have found NFmiFileSystemTest.cpp");

  result = FileComplete("NFmiFileSystem.cpp", "");
  if (result != "NFmiFileSystem.cpp")
    TEST_FAILED("Should not have found NFmiFileSystem from empty path");

  result = FileComplete("NFmiFileSystem.cpp", "../newbase");
  if (result != "../newbase/NFmiFileSystem.cpp")
    TEST_FAILED("Should have found NFmiFileSystem.cpp from ../newbase");

  result = FileComplete("NFmiFileSystem.cpp", "../test:../newbase");
  if (result != "../newbase/NFmiFileSystem.cpp")
    TEST_FAILED("Should have found NFmiFileSystem.cpp from ../test:../source");

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Tests the TemporaryFile function
 */
// ----------------------------------------------------------------------

void temporaryfile(void)
{
  using namespace std;
  using NFmiFileSystem::TemporaryFile;

  string result;

  result = TemporaryFile(".");
  if (result.empty())
    TEST_FAILED("Failed to create filename in .");

  result = TemporaryFile("/tmp");
  if (result.empty())
    TEST_FAILED("Failed to create filename in /tmp");

  try
  {
    result = TemporaryFile("/blah");
    TEST_FAILED("Should have failed to create filename in /blah");
  }
  catch (...)
  {
  }

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Tests the DirName function
 */
// ----------------------------------------------------------------------

void dirname(void)
{
  using namespace std;
  using NFmiFileSystem::DirName;

  string result;

  if ((result = DirName("")) != ".")
    TEST_FAILED("For '' should get '.', not '" + result + "'");

  if ((result = DirName(".")) != ".")
    TEST_FAILED("For '.' should get '.', not '" + result + "'");

  if ((result = DirName("/")) != "/")
    TEST_FAILED("For '/' should get '/', not '" + result + "'");

  if ((result = DirName("/aa")) != "/")
    TEST_FAILED("For '/aa' should get '/', not '" + result + "'");

  if ((result = DirName("/aa/")) != "/")
    TEST_FAILED("For '/aa/' should get '/', not '" + result + "'");

  if ((result = DirName("/aa/bb")) != "/aa")
    TEST_FAILED("For '/aa/bb' should get '/aa', not '" + result + "'");

  if ((result = DirName("/aa/bb/")) != "/aa")
    TEST_FAILED("For '/aa/bb/' should get '/aa', not '" + result + "'");

  if ((result = DirName("/aa/bb/cc")) != "/aa/bb")
    TEST_FAILED("For '/aa/bb/cc' should get '/aa/bb', not '" + result + "'");

  if ((result = DirName("aa")) != ".")
    TEST_FAILED("For 'aa' should get '.', not '" + result + "'");

  if ((result = DirName("aa/")) != ".")
    TEST_FAILED("For 'aa/' should get '.', not '" + result + "'");

  if ((result = DirName("aa/bb")) != "aa")
    TEST_FAILED("For 'aa/bb' should get 'aa', not '" + result + "'");

  if ((result = DirName("aa/bb/")) != "aa")
    TEST_FAILED("For 'aa/bb/' should get 'aa', not '" + result + "'");

  if ((result = DirName("aa/bb/cc")) != "aa/bb")
    TEST_FAILED("For 'aa/bb/cc' should get 'aa/bb', not '" + result + "'");

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Tests the BaseName function
 */
// ----------------------------------------------------------------------

void basename(void)
{
  using namespace std;
  using NFmiFileSystem::BaseName;

  string result;

  if ((result = BaseName("")) != "")
    TEST_FAILED("For '' should get '', not '" + result + "'");

  if ((result = BaseName(".")) != ".")
    TEST_FAILED("For '.' should get '.', not '" + result + "'");

  if ((result = BaseName("/")) != "/")
    TEST_FAILED("For '/' should get '/', not '" + result + "'");

  if ((result = BaseName("/aa")) != "aa")
    TEST_FAILED("For '/aa' should get 'aa', not '" + result + "'");

  if ((result = BaseName("/aa/")) != "aa")
    TEST_FAILED("For '/aa/' should get 'aa', not '" + result + "'");

  if ((result = BaseName("/aa/bb")) != "bb")
    TEST_FAILED("For '/aa/bb' should get 'bb', not '" + result + "'");

  if ((result = BaseName("/aa/bb/")) != "bb")
    TEST_FAILED("For '/aa/bb/' should get 'bb', not '" + result + "'");

  if ((result = BaseName("/aa/bb/cc")) != "cc")
    TEST_FAILED("For '/aa/bb/cc' should get 'cc', not '" + result + "'");

  if ((result = BaseName("aa")) != "aa")
    TEST_FAILED("For 'aa' should get 'aa', not '" + result + "'");

  if ((result = BaseName("aa/")) != "aa")
    TEST_FAILED("For 'aa/' should get 'aa', not '" + result + "'");

  if ((result = BaseName("aa/bb")) != "bb")
    TEST_FAILED("For 'aa/bb' should get 'bb', not '" + result + "'");

  if ((result = BaseName("aa/bb/")) != "bb")
    TEST_FAILED("For 'aa/bb/' should get 'bb', not '" + result + "'");

  if ((result = BaseName("aa/bb/cc")) != "cc")
    TEST_FAILED("For 'aa/bb/cc' should get 'cc', not '" + result + "'");

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test DirectoryFiles
 */
// ----------------------------------------------------------------------

void directoryfiles()
{
  std::string dir = "/tmp/NFmiDirectoryFilesDir";
  std::string cmd0 = "mkdir -p " + dir;
  std::string cmd1 = "touch " + dir + "/abba.txt";
  std::string cmd2 = "touch " + dir + "/akka.txt";
  std::string cmd3 = "touch " + dir + "/babba.txt";
  std::string cmd4 = "touch " + dir + "/kappa.txt";
  std::string cmdx = "touch " + dir + "/.kappa.txt";

  system(cmd0.c_str());
  system(cmd1.c_str());
  system(cmd2.c_str());
  system(cmd3.c_str());
  system(cmd4.c_str());
  system(cmdx.c_str());

  std::list<std::string> result = NFmiFileSystem::DirectoryFiles(dir);

  std::string cmd = "rm -rf " + dir;
  system(cmd.c_str());

  if (result.size() != 4)
    TEST_FAILED("Failed to find 4 regular files from the test directory, found " +
                Fmi::to_string(result.size()));

  // The following test was wrong
  // From
  // http://www.boost.org/doc/libs/1_56_0/libs/filesystem/doc/reference.html#Class-directory_iterator
  // -> "The order of directory entries obtained by dereferencing successive increments of a
  // directory_iterator is unspecified."

  // if(result.front() != "kappa.txt")
  //   TEST_FAILED("First result should be 'kappa.txt', not "+result.front());

  // if(result.back() != "abba.txt")
  //   TEST_FAILED("First result should be 'abba.txt', not "+result.back());

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test FindFile
 */
// ----------------------------------------------------------------------

void findfile()
{
  std::string dir = "/tmp/NFmiFileSystemTestDir";
  std::string cmd1 = "mkdir -p " + dir;
  std::string cmd2 = "touch " + dir + "/abba.txt";
  std::string cmd3 = "touch " + dir + "/akka.txt";
  std::string cmd4 = "touch " + dir + "/babba.txt";
  std::string cmd5 = "touch " + dir + "/kappa.txt";
  system(cmd1.c_str());
  system(cmd2.c_str());
  sleep(1);
  system(cmd3.c_str());
  sleep(1);
  system(cmd4.c_str());
  sleep(1);
  system(cmd5.c_str());
  sleep(1);

  std::string result;

  NFmiFileSystem::FindFile(dir + "/akka.txt", true, &result);
  if (result != "akka.txt")
    TEST_FAILED("akka.txt should have matched akka.txt, not " + result);

  NFmiFileSystem::FindFile(dir + "/a??a.txt", true, &result);
  if (result != "akka.txt")
    TEST_FAILED("a??a.txt should have matched akka.txt, not " + result);

  NFmiFileSystem::FindFile(dir + "/a??a.txt", false, &result);
  if (result != "abba.txt")
    TEST_FAILED("a??a.txt should have matched abba.txt, not " + result);

  NFmiFileSystem::FindFile(dir + "/a*.txt", true, &result);
  if (result != "akka.txt")
    TEST_FAILED("a*.txt should have matched akka.txt, not " + result);

  NFmiFileSystem::FindFile(dir + "/*a.txt", true, &result);
  if (result != "kappa.txt")
    TEST_FAILED("a*.txt should have matched kappa.txt, not " + result);

  NFmiFileSystem::FindFile(dir + "/*b*a.txt", true, &result);
  if (result != "babba.txt")
    TEST_FAILED("*b*a.txt should have matched babba.txt, not " + result);

  std::string cmd = "rm -rf " + dir;
  system(cmd.c_str());

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test PatternFiles
 */
// ----------------------------------------------------------------------

void patternfiles()
{
  std::string dir = "/tmp/NFmiFileSystemTestDir";
  std::string cmd0 = "rm -rf " + dir;
  std::string cmd1 = "mkdir -p " + dir;
  std::string cmd2 = "touch " + dir + "/abba.txt";
  std::string cmd3 = "touch " + dir + "/akka.txt";
  std::string cmd4 = "touch " + dir + "/babba.txt";
  std::string cmd5 = "touch " + dir + "/kappa.txt";
  std::string cmd6 = "mkdir -p " + dir + "/adda.txt";  // dirs must not match
  system(cmd0.c_str());
  system(cmd1.c_str());
  system(cmd2.c_str());
  system(cmd3.c_str());
  system(cmd4.c_str());
  system(cmd5.c_str());

  std::list<std::string> out;

  out = NFmiFileSystem::PatternFiles(dir + "/akka.txt");
  if (out.size() != 1)
    TEST_FAILED("akka.txt should have matched exactly 1 file, not " +
                NFmiStringTools::Convert(out.size()));
  if (out.front() != "akka.txt")
    TEST_FAILED("First match for akka.txt should have been akka.txt, not " + out.front());

  out = NFmiFileSystem::PatternFiles(dir + "/a??a.txt");
  if (out.size() != 2)
    TEST_FAILED("a??a.txt should have matched 2 files");

  out = NFmiFileSystem::PatternFiles(dir + "/a*.txt");
  if (out.size() != 2)
    TEST_FAILED("a*.txt should have matched 2 files");

  out = NFmiFileSystem::PatternFiles(dir + "/*a.txt");
  if (out.size() != 4)
    TEST_FAILED("*a.txt should have matched 4 files");

  out = NFmiFileSystem::PatternFiles(dir + "/*b*a.txt");
  if (out.size() != 2)
    TEST_FAILED("*b*a.txt should have matched 2 files");

  std::string cmd = "rm -rf " + dir;
  system(cmd.c_str());

  // Test also without path component

  out = NFmiFileSystem::PatternFiles("*.cpp");
  if (out.size() < 20)
    TEST_FAILED("*.cpp should have matched atleast 20 files");

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test Directories
 */
// ----------------------------------------------------------------------

void directories()
{
  std::string dir = "/tmp/NFmiFileSystemTestDir";
  std::string cmd0 = "rm -rf " + dir;
  std::string cmd1 = "mkdir -p " + dir;
  std::string cmd2 = "mkdir -p " + dir + "/abba.txt";
  std::string cmd3 = "mkdir -p " + dir + "/akka.txt";
  std::string cmd4 = "mkdir -p " + dir + "/babba.txt";
  std::string cmd5 = "mkdir -p " + dir + "/kappa.txt";
  std::string cmd6 = "touch " + dir + "/adda.txt";  // files must not match
  system(cmd0.c_str());
  system(cmd1.c_str());
  system(cmd2.c_str());
  system(cmd3.c_str());
  system(cmd4.c_str());
  system(cmd5.c_str());

  std::list<std::string> out;

  out = NFmiFileSystem::Directories(dir);
  if (out.size() != 4)
    TEST_FAILED("should have matched exactly 4 directories, not " +
                NFmiStringTools::Convert(out.size()));

  // This test is wrong - the function gives no guarantees on ordering
  // From
  // http://www.boost.org/doc/libs/1_56_0/libs/filesystem/doc/reference.html#Class-directory_iterator
  // -> "The order of directory entries obtained by dereferencing successive increments of a
  // directory_iterator is unspecified

  // if(out.front() != "kappa.txt")
  //   TEST_FAILED("First match should have been kappa.txt, not "+out.front());

  std::string cmd = "rm -rf " + dir;
  system(cmd.c_str());

  TEST_PASSED();
}

void compression_detection()
{
  const auto check = [](bool expected, const std::string& name) -> void {
      bool ok =  Fmi::is_compressed(name);
    if (ok ^ expected)
      TEST_FAILED(name + " misdetected as " + (expected ? "not" : "") + " compressed");
  };

  check(true, "foo.gz");
  check(true, "foo.bz2");
#ifndef WIN32
  check(true, "foo.xz");
  check(true, "foo.zstd");
#endif 
  check(false, "foo");
  check(false, "foo.bar");
  check(false, "");

  TEST_PASSED();
}

void query_data_detection()
{
  const auto check = [](bool expected, const std::string& name) -> void {
    bool ok =  NFmiFileSystem::IsQueryData(name);
    if (ok ^ expected)
      TEST_FAILED(name + " misdetected as " + (expected ? "not" : "") + " query data");
  };

  check(false, "");
  check(false, "foo");
  check(false, "foo.bar");
  check(true, "foo.sqd");
  check(true, "foo.fqd");
  check(false, "foo.gz");
  check(false, "foo.bz2");
  check(true, "foo.sqd.gz");
  check(true, "foo.sqd.bz2");
  check(true, "foo.fqd.gz");
  check(true, "foo.fqd.bz2");
#ifndef WIN32
  check(false, "foo.xz");
  check(false, "foo.zstd");
  check(true, "foo.sqd.xz");
  check(true, "foo.sqd.zstd");
  check(true, "foo.fqd.xz");
  check(true, "foo.fqd.zstd");
#endif

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
    TEST(directoryfiles);
    TEST(filecomplete);
    TEST(temporaryfile);
    TEST(dirname);
    TEST(basename);
    TEST(patternfiles);
    TEST(directories);
    TEST(findfile);
    TEST(compression_detection);
    TEST(query_data_detection);
  }
};

}  // namespace NFmiFileSystemTest

//! The main program
int main(void)
{
  using namespace std;
  cout << endl << "NFmiFileSystem tester" << endl << "=====================" << endl;
  NFmiFileSystemTest::tests t;
  return t.run();
}

// ======================================================================
