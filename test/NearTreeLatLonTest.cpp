// ======================================================================
/*!
 * \file
 * \brief Regression tests for Fmi::NearTreeLatLon
 */
// ======================================================================

#include "NearTree.h"
#include "NearTreeLatLon.h"
#include <boost/lexical_cast.hpp>
#include <regression/tframe.h>
#include <iostream>
#include <sstream>
#include <string>

using namespace std;
using namespace Fmi;

using NearTreeType = NearTreeLatLon<int>;

NearTree<NearTreeType, NearTreeLatLonDistance<NearTreeType>> tree;

const int Helsinki = 1;
const int Tampere = 2;
const int Turku = 3;
const int Oulu = 4;
const int Kuopio = 5;
const int Rovaniemi = 6;
const int Hanko = 7;
const int Utsjoki = 8;

map<int, string> names = {make_pair(Helsinki, "Helsinki"),
                          make_pair(Tampere, "Tampere"),
                          make_pair(Turku, "Turku"),
                          make_pair(Oulu, "Oulu"),
                          make_pair(Kuopio, "Kuopio"),
                          make_pair(Rovaniemi, "Rovaniemi"),
                          make_pair(Hanko, "Hanko"),
                          make_pair(Utsjoki, "Utsjoki")};

void prepare()
{
  tree.insert(NearTreeType(24.9354, 60.1695, Helsinki));
  tree.insert(NearTreeType(23.8702, 61.6074, Tampere));
  tree.insert(NearTreeType(22.3333, 60.5333, Turku));
  tree.insert(NearTreeType(25.9694, 65.2173, Oulu));
  tree.insert(NearTreeType(27.8957, 62.9472, Kuopio));
  tree.insert(NearTreeType(25.9525, 66.60, Rovaniemi));
  tree.insert(NearTreeType(22.95, 59.833, Hanko));
  tree.insert(NearTreeType(27.0284, 69.908, Utsjoki));
  tree.flush();
}

namespace TreeTest
{
// ----------------------------------------------------------------------

void size()
{
  if (tree.size() != 8)
    TEST_FAILED("Test tree size should be 8, not " + boost::lexical_cast<string>(tree.size()));
  TEST_PASSED();
}

// ----------------------------------------------------------------------

void empty()
{
  if (tree.empty())
    TEST_FAILED("Test tree should not be empty");
  TEST_PASSED();
}

// ----------------------------------------------------------------------

void nearest()
{
  {
    auto ret = tree.nearest(NearTreeType(25, 60));
    if (!ret)
      TEST_FAILED("Failed to find nearest point for 25,60");
    if (names.at(ret->ID()) != "Helsinki")
      TEST_FAILED("Closest point to 25,60 should be Helsinki, not " + names.at(ret->ID()));
  }

  {
    auto ret = tree.nearest(NearTreeType(0, 0));
    if (!ret)
      TEST_FAILED("Failed to find nearest point for 0,0");
    if (names.at(ret->ID()) != "Hanko")
      TEST_FAILED("Closest point to 0,0 should be Hanko, not " + names.at(ret->ID()));
  }

  {
    auto ret = tree.nearest(NearTreeType(25, 80));
    if (!ret)
      TEST_FAILED("Failed to find nearest point for 25,80");
    if (names.at(ret->ID()) != "Utsjoki")
      TEST_FAILED("Closest point to 25,80 should be Utsjoki, not " + names.at(ret->ID()));
  }

  TEST_PASSED();
}

// ----------------------------------------------------------------------

void farthest()
{
  {
    auto ret = tree.farthest(NearTreeType(0, 0));
    if (!ret)
      TEST_FAILED("Failed to find farthest point for 0,0");
    if (names.at(ret->ID()) != "Utsjoki")
      TEST_FAILED("Farthest point to 0,0 should be Utsjoki, not " + names.at(ret->ID()));
  }

  {
    auto ret = tree.farthest(NearTreeType(25, 80));
    if (!ret)
      TEST_FAILED("Failed to find farthest point for 25,80");
    if (names.at(ret->ID()) != "Hanko")
      TEST_FAILED("Farthest point to 25,80 should be Hanko, not " + names.at(ret->ID()));
  }

  TEST_PASSED();
}

// ----------------------------------------------------------------------

void nearest_many()
{
  auto ret = tree.nearestones(NearTreeType(25, 60), NearTreeType::ChordLength(200));
  if (ret.size() != 4)
  {
    // print(ret);
    TEST_FAILED("Failed to find 4 nearest points for 25,60 with radius 200 km");
  }

  ret = tree.nearestones(NearTreeType(25, 70), NearTreeType::ChordLength(400));
  if (ret.size() != 2)
  {
    // print(ret);
    TEST_FAILED("Failed to find 2 nearest points for 25,70 with radius 400 km");
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
    TEST(size);
    TEST(empty);
    TEST(nearest);
    TEST(farthest);
    TEST(nearest_many);
  }
};

}  // namespace TreeTest

//! The main program
int main(void)
{
  prepare();

  using namespace std;
  cout << endl << "Tree" << endl << "==========" << endl;
  TreeTest::tests t;
  return t.run();
}

// ======================================================================
