// ======================================================================
/*!
 * \file
 * \brief Regression tests for Fmi::TernarySearchTree
 */
// ======================================================================

#include "TernarySearchTree.h"
#include <boost/lexical_cast.hpp>
#include <boost/locale.hpp>
#include <regression/tframe.h>
#include <fstream>
#include <memory>
#include <string>

namespace TernarySearchTreeTest
{
// ----------------------------------------------------------------------

void insert()
{
  Fmi::TernarySearchTree<std::string> tree;

  std::shared_ptr<std::string> foo = std::make_shared<std::string>("not empty now");

  if (!tree.insert("ii", foo))
    TEST_FAILED("Failed to insert ii");

  if (tree.insert("ii", foo))
    TEST_FAILED("Succeded to insert ii twice");

  if (!tree.insert("imatra", foo))
    TEST_FAILED("Failed to insert imatra");

  if (tree.insert("imatra", foo))
    TEST_FAILED("Succeded to insert imatra twice");

  TEST_PASSED();
}

// ----------------------------------------------------------------------

void insert_copy()
{
  Fmi::TernarySearchTree<std::string> tree;

  std::string foo = "foobar";

  if (!tree.insert("ii", foo))
    TEST_FAILED("Failed to insert ii");

  if (tree.insert("ii", foo))
    TEST_FAILED("Succeded to insert ii twice");

  if (!tree.insert("imatra", foo))
    TEST_FAILED("Failed to insert imatra");

  if (tree.insert("imatra", foo))
    TEST_FAILED("Succeded to insert imatra twice");

  TEST_PASSED();
}

// ----------------------------------------------------------------------

void find()
{
  Fmi::TernarySearchTree<std::string> tree;
  typedef Fmi::TernarySearchTree<std::string>::element_type element_type;

  element_type res;

  element_type imatra = std::make_shared<std::string>("imatra");
  element_type ii = std::make_shared<std::string>("ii");
  element_type iisalmi = std::make_shared<std::string>("iisalmi");

  tree.insert("imatra", imatra);
  tree.insert("ii", ii);
  tree.insert("iisalmi", iisalmi);

  res = tree.find("X");
  if (res != nullptr)
    TEST_FAILED("Should not have found X");

  res = tree.find("imatra");
  if (res == nullptr)
    TEST_FAILED("Should have found imatra");
  if (*res != "imatra")
    TEST_FAILED("Should have found imatra result, found " + *res + " instead");

  res = tree.find("imatraX");
  if (res != nullptr)
    TEST_FAILED("Should not have found imatraX");

  res = tree.find("Imatr");
  if (res != nullptr)
    TEST_FAILED("Should not have found Imatr");

  TEST_PASSED();
}

// ----------------------------------------------------------------------

void findprefix()
{
  Fmi::TernarySearchTree<std::string> tree;
  Fmi::TernarySearchTree<std::string>::result_type res;
  typedef Fmi::TernarySearchTree<std::string>::element_type element_type;

  element_type imatra = std::make_shared<std::string>("imatra");
  element_type ii = std::make_shared<std::string>("ii");
  element_type iisalmi = std::make_shared<std::string>("iisalmi");

  tree.insert("imatra", imatra);
  tree.insert("ii", ii);
  tree.insert("iisalmi", iisalmi);

  res = tree.findprefix("X");
  if (res.size() != 0)
    TEST_FAILED("Should not have found X");

  res = tree.findprefix("imatra");
  if (res.size() != 1)
    TEST_FAILED("Should have found imatra");

  res = tree.findprefix("imatraX");
  if (res.size() != 0)
    TEST_FAILED("Should not have found imatraX");

  res = tree.findprefix("ii");
  if (res.size() != 2)
    TEST_FAILED("Should have found ii and iisalmi");

  res = tree.findprefix("i");
  if (res.size() != 3)
    TEST_FAILED("Should have found imatra, ii and iisalmi");

  TEST_PASSED();
}

void wordlist()
{
  Fmi::TernarySearchTree<std::string> tree;
  typedef Fmi::TernarySearchTree<std::string>::element_type element_type;

  std::ifstream in("data/words.txt");
  if (!in)
    TEST_FAILED("Failed to open data/words.txt for reading");

  int count = 0;
  element_type dummy = std::make_shared<std::string>("foobar");

  std::string word;
  std::string collated;
  char chr;
  while (getline(in, word))
  {
    collated.clear();
    while (in.get(chr), chr != '\0')
      collated += chr;
    in.get(chr);  // skip newline, and ignore the zero-byte

    tree.insert(collated, dummy);
    element_type res = tree.find(collated);
    if (!res)
      TEST_FAILED("Failed to find word '" + word + "' from tree after " +
                  boost::lexical_cast<std::string>(count) + " insertions");
    ++count;
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
    TEST(insert);
    TEST(insert_copy);
    TEST(find);
    TEST(findprefix);
    TEST(wordlist);
  }
};

}  // namespace TernarySearchTreeTest

//! The main program
int main(void)
{
  using namespace std;
  cout << std::endl << "TernarySearchTree" << endl << "=================" << endl;
  TernarySearchTreeTest::tests t;
  return t.run();
}

// ======================================================================
