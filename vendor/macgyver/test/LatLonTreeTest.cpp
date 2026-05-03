// ======================================================================
/*!
 * \file
 * \brief Regression tests for Fmi::LatLonTree
 */
// ======================================================================

#include "LatLonTree.h"
#include <boost/lexical_cast.hpp>
#include <regression/tframe.h>
#include <iostream>
#include <sstream>
#include <string>

using namespace std;

struct Point
{
  Point() : longitude(0), latitude(0), name() {}
  Point(double x, double y, const std::string& n = "") : longitude(x), latitude(y), name(n) {}
  bool operator==(const Point& other) const
  {
    return (longitude == other.longitude && latitude == other.latitude);
  }

  bool operator!=(const Point& other) const { return !(operator==(other)); }
  double longitude;
  double latitude;
  std::string name;
};

template <typename T>
void print(const T& points)
{
  for (const auto& p : points)
    std::cout << p.second.name << " " << p.first << std::endl;
  std::cout << std::endl;
}

Fmi::LatLonTree<Point> latlontree;

void prepare()
{
  latlontree.insert(Point(24.9354, 60.1695, "Helsinki"));
  latlontree.insert(Point(23.8702, 61.6074, "Tampere"));
  latlontree.insert(Point(22.3333, 60.5333, "Turku"));
  latlontree.insert(Point(25.9694, 65.2173, "Oulu"));
  latlontree.insert(Point(27.8957, 62.9472, "Kuopio"));
  latlontree.insert(Point(25.9525, 66.60, "Rovaniemi"));
  latlontree.insert(Point(22.95, 59.833, "Hanko"));
  latlontree.insert(Point(27.0284, 69.908, "Utsjoki"));
}

namespace LatLonTreeTest
{
// ----------------------------------------------------------------------

void size()
{
  if (latlontree.size() != 8)
    TEST_FAILED("Test tree size should be 8, not " +
                boost::lexical_cast<string>(latlontree.size()));
  TEST_PASSED();
}

// ----------------------------------------------------------------------

void empty()
{
  if (latlontree.empty())
    TEST_FAILED("Test tree should not be empty");
  TEST_PASSED();
}

// ----------------------------------------------------------------------

void nearest()
{
  {
    auto ret = latlontree.nearest(Point(25, 60));
    if (!ret)
      TEST_FAILED("Failed to find nearest point for 25,60");
    if (ret->name != "Helsinki")
      TEST_FAILED("Closest point to 25,60 should be Helsinki, not " + ret->name);
  }

  {
    auto ret = latlontree.nearest(Point(0, 0));
    if (!ret)
      TEST_FAILED("Failed to find nearest point for 0,0");
    if (ret->name != "Hanko")
      TEST_FAILED("Closest point to 0,0 should be Hanko, not " + ret->name);
  }

  {
    auto ret = latlontree.nearest(Point(25, 80));
    if (!ret)
      TEST_FAILED("Failed to find nearest point for 25,80");
    if (ret->name != "Utsjoki")
      TEST_FAILED("Closest point to 25,80 should be Utsjoki, not " + ret->name);
  }

  TEST_PASSED();
}

// ----------------------------------------------------------------------

void farthest()
{
  {
    auto ret = latlontree.farthest(Point(0, 0));
    if (!ret)
      TEST_FAILED("Failed to find farthest point for 0,0");
    if (ret->name != "Utsjoki")
      TEST_FAILED("Farthest point to 0,0 should be Utsjoki, not " + ret->name);
  }

  {
    auto ret = latlontree.farthest(Point(25, 80));
    if (!ret)
      TEST_FAILED("Failed to find farthest point for 25,80");
    if (ret->name != "Hanko")
      TEST_FAILED("Farthest point to 25,80 should be Hanko, not " + ret->name);
  }

  TEST_PASSED();
}

// ----------------------------------------------------------------------

void nearest_many()
{
  auto ret = latlontree.nearestones(Point(25, 60), 200);
  if (ret.size() != 4)
  {
    // print(ret);
    TEST_FAILED("Failed to find 4 nearest points for 25,60 with radius 200 km");
  }

  ret = latlontree.nearestones(Point(25, 70), 400);
  if (ret.size() != 2)
  {
    // print(ret);
    TEST_FAILED("Failed to find 2 nearest points for 25,70 with radius 400");
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

}  // namespace LatLonTreeTest

//! The main program
int main(void)
{
  prepare();

  using namespace std;
  cout << endl << "LatLonTree" << endl << "==========" << endl;
  LatLonTreeTest::tests t;
  return t.run();
}

// ======================================================================
