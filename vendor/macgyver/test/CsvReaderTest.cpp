#include "CsvReader.h"
#include "StringConversion.h"
#include <regression/tframe.h>

using namespace std;
using namespace std::placeholders;

namespace CsvReaderTest
{
typedef vector<Fmi::CsvReader::row_type> Table;

struct TableBuilder
{
  void addrow(const Fmi::CsvReader::row_type& row) { table.push_back(row); }
  Table table;
};

ostream& operator<<(ostream& os, const TableBuilder& tb)
{
  for (unsigned int row = 0; row < tb.table.size(); ++row)
  {
    os << "Row " << row << ':';
    for (unsigned int col = 0; col < tb.table[row].size(); ++col)
    {
      if (col > 0)
        os << ',';
      os << '"' << tb.table[row][col] << '"';
    }
    os << endl;
  }
  return os;
}

template <typename T>
string tostr(const T& tmp)
{
  return Fmi::to_string(tmp);
}

// ----------------------------------------------------------------------

void trimming()
{
  TableBuilder tb;
  Fmi::CsvReader::read("data/test_01.csv", std::bind(&TableBuilder::addrow, &tb, _1));

  // cout << tb;

  if (tb.table.size() != 1)
    TEST_FAILED("File should contain one row, not " + tostr(tb.table.size()));
  if (tb.table[0].size() != 5)
    TEST_FAILED("File should contain 5 fields, not " + tostr(tb.table[0].size()));
  if (tb.table[0][0] != "1")
    TEST_FAILED("Field 1 should be '1', not '" + tb.table[0][0] + "'");
  if (tb.table[0][1] != "2")
    TEST_FAILED("Field 2 should be '2', not '" + tb.table[0][1] + "'");
  if (tb.table[0][2] != "3")
    TEST_FAILED("Field 3 should be '3', not '" + tb.table[0][2] + "'");
  if (tb.table[0][3] != "4")
    TEST_FAILED("Field 4 should be '4', not '" + tb.table[0][3] + "'");
  if (tb.table[0][4] != "5")
    TEST_FAILED("Field 5 should be '5', not '" + tb.table[0][4] + "'");

  TEST_PASSED();
}

// ----------------------------------------------------------------------

void emptyfields()
{
  TableBuilder tb;
  Fmi::CsvReader::read("data/test_02.csv", std::bind(&TableBuilder::addrow, &tb, _1));

  // cout << tb;

  if (tb.table.size() != 1)
    TEST_FAILED("File should contain one row, not " + tostr(tb.table.size()));
  if (tb.table[0].size() != 6)
    TEST_FAILED("File should contain 6 fields, not " + tostr(tb.table[0].size()));
  if (tb.table[0][0] != "")
    TEST_FAILED("Field 1 should be '', not '" + tb.table[0][0] + "'");
  if (tb.table[0][1] != "")
    TEST_FAILED("Field 2 should be '', not '" + tb.table[0][1] + "'");
  if (tb.table[0][2] != "")
    TEST_FAILED("Field 3 should be '', not '" + tb.table[0][2] + "'");
  if (tb.table[0][3] != "")
    TEST_FAILED("Field 4 should be '', not '" + tb.table[0][3] + "'");
  if (tb.table[0][4] != "")
    TEST_FAILED("Field 5 should be '', not '" + tb.table[0][4] + "'");

  TEST_PASSED();
}

// ----------------------------------------------------------------------

void quotes()
{
  TableBuilder tb;
  Fmi::CsvReader::read("data/test_03.csv", std::bind(&TableBuilder::addrow, &tb, _1));

  // cout << tb;

  if (tb.table.size() != 1)
    TEST_FAILED("File should contain one row, not " + tostr(tb.table.size()));
  if (tb.table[0].size() != 3)
    TEST_FAILED("File should contain 3 fields, not " + tostr(tb.table[0].size()));
  if (tb.table[0][0] != ",")
    TEST_FAILED("Field 1 should be ',', not '" + tb.table[0][0] + "'");
  if (tb.table[0][1] != ",")
    TEST_FAILED("Field 2 should be ',', not '" + tb.table[0][1] + "'");
  if (tb.table[0][2] != "")
    TEST_FAILED("Field 3 should be '', not '" + tb.table[0][2] + "'");

  TEST_PASSED();
}

// ----------------------------------------------------------------------

void newlines()
{
  TableBuilder tb;
  Fmi::CsvReader::read("data/test_04.csv", std::bind(&TableBuilder::addrow, &tb, _1));

  // cout << tb;

  if (tb.table.size() != 1)
    TEST_FAILED("File should contain one row, not " + tostr(tb.table.size()));
  if (tb.table[0].size() != 1)
    TEST_FAILED("File should contain 1 field, not " + tostr(tb.table[0].size()));

  TEST_PASSED();
}

// ----------------------------------------------------------------------

void doublequotes()
{
  TableBuilder tb;
  Fmi::CsvReader::read("data/test_05.csv", std::bind(&TableBuilder::addrow, &tb, _1));

  // cout << tb;

  if (tb.table.size() != 1)
    TEST_FAILED("File should contain one row, not " + tostr(tb.table.size()));
  if (tb.table[0].size() != 6)
    TEST_FAILED("File should contain 6 fields, not " + tostr(tb.table[0].size()));
  if (tb.table[0][0] != "\"a,b\"")
    TEST_FAILED("Field 1 should be '\"a,b\"', not '" + tb.table[0][0] + "'");
  if (tb.table[0][1] != "")
    TEST_FAILED("Field 2 should be '', not '" + tb.table[0][1] + "'");
  if (tb.table[0][2] != " \"\" ")
    TEST_FAILED("Field 3 should be ' \"\" ', not '" + tb.table[0][2] + "'");
  if (tb.table[0][3] != "\"\" ")
    TEST_FAILED("Field 4 should be '\"\" ', not '" + tb.table[0][3] + "'");
  if (tb.table[0][4] != " \"\"")
    TEST_FAILED("Field 5 should be ' \"\"', not '" + tb.table[0][4] + "'");
  if (tb.table[0][5] != "\"\"")
    TEST_FAILED("Field 6 should be '\"\"', not '" + tb.table[0][4] + "'");

  TEST_PASSED();
}

// ----------------------------------------------------------------------

void notrimming()
{
  TableBuilder tb;
  Fmi::CsvReader::read("data/test_06.csv", std::bind(&TableBuilder::addrow, &tb, _1));

  // cout << tb;

  if (tb.table.size() != 1)
    TEST_FAILED("File should contain one row, not " + tostr(tb.table.size()));
  if (tb.table[0].size() != 3)
    TEST_FAILED("File should contain 3 fields, not " + tostr(tb.table[0].size()));
  if (tb.table[0][0] != " a, b ,c ")
    TEST_FAILED("Field 1 should be ' a, b ,c ', not '" + tb.table[0][0] + "'");
  if (tb.table[0][1] != "a b  c")
    TEST_FAILED("Field 2 should be 'a b  c', not '" + tb.table[0][1] + "'");
  if (tb.table[0][2] != "")
    TEST_FAILED("Field 3 should be '', not '" + tb.table[0][1] + "'");

  TEST_PASSED();
}

// ----------------------------------------------------------------------

void trimerror1()
{
  TableBuilder tb;

  try
  {
    Fmi::CsvReader::read("data/test_07.csv", std::bind(&TableBuilder::addrow, &tb, _1));
    TEST_FAILED("Should have errored due to undoubled quotes");
  }
  catch (...)
  {
  }

  TEST_PASSED();
}

// ----------------------------------------------------------------------

void trimerror2()
{
  TableBuilder tb;

  try
  {
    Fmi::CsvReader::read("data/test_08.csv", std::bind(&TableBuilder::addrow, &tb, _1));
    TEST_FAILED("Should have errored due to undoubled quotes");
  }
  catch (...)
  {
  }

  TEST_PASSED();
}

// ----------------------------------------------------------------------

void emptyfile()
{
  TableBuilder tb;

  Fmi::CsvReader::read("data/test_09.csv", std::bind(&TableBuilder::addrow, &tb, _1));

  if (tb.table.size() != 0)
    TEST_FAILED("File should contain 0 rows, not " + tostr(tb.table.size()));

  TEST_PASSED();
}

// ----------------------------------------------------------------------

void onefield()
{
  TableBuilder tb;
  Fmi::CsvReader::read("data/test_10.csv", std::bind(&TableBuilder::addrow, &tb, _1));

  // cout << tb;

  if (tb.table.size() != 1)
    TEST_FAILED("File should contain one row, not " + tostr(tb.table.size()));
  if (tb.table[0].size() != 1)
    TEST_FAILED("File should contain 1 field, not " + tostr(tb.table[0].size()));
  if (tb.table[0][0] != "a")
    TEST_FAILED("Field 1 should be 'a', not '" + tb.table[0][0] + "'");

  TEST_PASSED();
}

// ----------------------------------------------------------------------

void fourfields()
{
  TableBuilder tb;
  Fmi::CsvReader::read("data/test_11.csv", std::bind(&TableBuilder::addrow, &tb, _1));

  // cout << tb;

  if (tb.table.size() != 1)
    TEST_FAILED("File should contain one row, not " + tostr(tb.table.size()));
  if (tb.table[0].size() != 4)
    TEST_FAILED("File should contain 4 fields, not " + tostr(tb.table[0].size()));
  if (tb.table[0][0] != "1")
    TEST_FAILED("Field 1 should be '1', not '" + tb.table[0][0] + "'");
  if (tb.table[0][1] != "2")
    TEST_FAILED("Field 2 should be '2', not '" + tb.table[0][1] + "'");
  if (tb.table[0][2] != "3")
    TEST_FAILED("Field 3 should be '3', not '" + tb.table[0][2] + "'");
  if (tb.table[0][3] != "4")
    TEST_FAILED("Field 4 should be '4', not '" + tb.table[0][3] + "'");

  TEST_PASSED();
}

// ----------------------------------------------------------------------

void emptyrows()
{
  TableBuilder tb;

  Fmi::CsvReader::read("data/test_12.csv", std::bind(&TableBuilder::addrow, &tb, _1));

  if (tb.table.size() != 0)
    TEST_FAILED("File should contain 0 rows, not " + tostr(tb.table.size()));

  TEST_PASSED();
}

// ----------------------------------------------------------------------

void onequote()
{
  TableBuilder tb;
  Fmi::CsvReader::read("data/test_13.csv", std::bind(&TableBuilder::addrow, &tb, _1));

  // cout << tb;

  if (tb.table.size() != 1)
    TEST_FAILED("File should contain one row, not " + tostr(tb.table.size()));
  if (tb.table[0].size() != 1)
    TEST_FAILED("File should contain 1 field, not " + tostr(tb.table[0].size()));
  if (tb.table[0][0] != "abc")
    TEST_FAILED("Field 1 should be 'abc', not '" + tb.table[0][0] + "'");

  TEST_PASSED();
}

// ----------------------------------------------------------------------

void semicolon()
{
  TableBuilder tb;
  Fmi::CsvReader::read("data/test_14.csv", std::bind(&TableBuilder::addrow, &tb, _1), ';');

  // cout << tb;

  if (tb.table.size() != 1)
    TEST_FAILED("File should contain one row, not " + tostr(tb.table.size()));
  if (tb.table[0].size() != 5)
    TEST_FAILED("File should contain 5 fields, not " + tostr(tb.table[0].size()));
  if (tb.table[0][0] != "1")
    TEST_FAILED("Field 1 should be '1', not '" + tb.table[0][0] + "'");
  if (tb.table[0][1] != "2")
    TEST_FAILED("Field 2 should be '2', not '" + tb.table[0][1] + "'");
  if (tb.table[0][2] != "3")
    TEST_FAILED("Field 3 should be '3', not '" + tb.table[0][2] + "'");
  if (tb.table[0][3] != "4")
    TEST_FAILED("Field 4 should be '4', not '" + tb.table[0][3] + "'");
  if (tb.table[0][4] != "5")
    TEST_FAILED("Field 5 should be '5', not '" + tb.table[0][4] + "'");

  TEST_PASSED();
}

// ----------------------------------------------------------------------

void comments()
{
  TableBuilder tb;
  Fmi::CsvReader::read("data/test_15.csv", std::bind(&TableBuilder::addrow, &tb, _1));

  // cout << tb;

  if (tb.table.size() != 2)
    TEST_FAILED("File should contain two rows, not " + tostr(tb.table.size()));
  if (tb.table[0].size() != 5)
    TEST_FAILED("File should contain 5 fields, not " + tostr(tb.table[0].size()));
  if (tb.table[0][0] != "1")
    TEST_FAILED("Field 1 should be '1', not '" + tb.table[0][0] + "'");
  if (tb.table[0][1] != "2")
    TEST_FAILED("Field 2 should be '2', not '" + tb.table[0][1] + "'");
  if (tb.table[0][2] != "3")
    TEST_FAILED("Field 3 should be '3', not '" + tb.table[0][2] + "'");
  if (tb.table[0][3] != "4")
    TEST_FAILED("Field 4 should be '4', not '" + tb.table[0][3] + "'");
  if (tb.table[0][4] != "5")
    TEST_FAILED("Field 5 should be '5', not '" + tb.table[0][4] + "'");

  if (tb.table[1][0] != "1")
    TEST_FAILED("Field 1 should be '1', not '" + tb.table[1][0] + "'");
  if (tb.table[1][1] != "#")
    TEST_FAILED("Field 2 should be '#', not '" + tb.table[1][1] + "'");
  if (tb.table[1][2] != "3")
    TEST_FAILED("Field 3 should be '3', not '" + tb.table[1][2] + "'");

  TEST_PASSED();
}

// ----------------------------------------------------------------------

// The actual test driver
class tests : public tframe::tests
{
  //! Overridden message separator
  virtual const char* error_message_prefix() const { return "\n\t"; }
  //! Main test suite
  void test(void)
  {
    TEST(trimming);
    TEST(emptyfields);
    TEST(quotes);
    TEST(newlines);
    TEST(doublequotes);
    TEST(notrimming);
    TEST(trimerror1);
    TEST(trimerror2);
    TEST(emptyfile);
    TEST(onefield);
    TEST(fourfields);
    TEST(emptyrows);
    TEST(onequote);
    TEST(semicolon);
    TEST(comments);
  }

};  // class tests

}  // namespace CsvReaderTest

int main(void)
{
  cout << endl << "CsvReader tester" << endl << "================" << endl;
  CsvReaderTest::tests t;
  return t.run();
}
