// ======================================================================
/*!
 * \file
 * \brief Regression tests for Fmi::NearTree
 */
// ======================================================================

#include "NearTree.h"
#include <boost/lexical_cast.hpp>
#include <regression/tframe.h>
#include <iostream>
#include <sstream>
#include <string>

using namespace std;

struct Point
{
  Point() : x_(0), y_(0) {}
  Point(float x, float y) : x_(x), y_(y) {}
  std::string str() const
  {
    ostringstream out;
    out << x_ << "," << y_;
    return out.str();
  }

  bool operator==(const Point& other) const { return (x_ == other.x_ && y_ == other.y_); }
  bool operator!=(const Point& other) const { return !(operator==(other)); }
  float x_;
  float y_;
  float x() const { return x_; }
  float y() const { return y_; }
};

Fmi::NearTree<Point> ntree;

void prepare()
{
  for (int j = -1; j <= 1; j++)
    for (int i = -1; i <= 1; i++)
      ntree.insert(Point(i, j));
}

namespace NearTreeTest
{
// ----------------------------------------------------------------------

void size()
{
  if (ntree.size() != 9)
    TEST_FAILED("Test tree size should be 9, not " + boost::lexical_cast<string>(ntree.size()));
  TEST_PASSED();
}

// ----------------------------------------------------------------------

void empty()
{
  if (ntree.empty())
    TEST_FAILED("Test tree should not be empty");
  TEST_PASSED();
}

// ----------------------------------------------------------------------

void nearest()
{
  {
    auto ret = ntree.nearest(Point(1, 0));
    if (!ret)
      TEST_FAILED("Failed to find nearest point for 1,0");
    if (*ret != Point(1, 0))
      TEST_FAILED("Closest point to 1,0 should be 1,0, not " + ret->str());
  }

  {
    auto ret = ntree.nearest(Point(0.25, 0.1));
    if (!ret)
      TEST_FAILED("Failed to find nearest point for 0.25,0.1");
    if (*ret != Point(0, 0))
      TEST_FAILED("Closest point to 0.25,0.1 should be 0,0, not " + ret->str());
  }

  {
    auto ret = ntree.nearest(Point(1.25, -1.1));
    if (!ret)
      TEST_FAILED("Failed to find nearest point for 1.25,-1.1");
    if (*ret != Point(1, -1))
      TEST_FAILED("Closest point to 1.25,-1.1 should be 1,-1, not " + ret->str());
  }

  TEST_PASSED();
}

// ----------------------------------------------------------------------

void nearest_distancelimit()
{
  {
    auto ret = ntree.nearest(Point(1, 0), 0.1);
    if (!ret)
      TEST_FAILED("Failed to find nearest point for 1,0");
    if (*ret != Point(1, 0))
      TEST_FAILED("Closest point to 1,0 should be 1,0, not " + ret->str());
  }

  {
    auto ret = ntree.nearest(Point(0.25, 0.1), 0.5);
    if (!ret)
      TEST_FAILED("Failed to find nearest point for 0.25,0.1 with distance limit 0.5");
    if (*ret != Point(0, 0))
      TEST_FAILED("Closest point to 0.25,0.1 should be 0,0, not " + ret->str());
  }

  {
    auto ret = ntree.nearest(Point(0.25, 0.1), 0.1);
    if (ret)
      TEST_FAILED("Should not find a point for 0.25,0.1 with distance limit 0.1");
  }

  TEST_PASSED();
}

// ----------------------------------------------------------------------

void farthest()
{
  {
    auto ret = ntree.farthest(Point(1, 1));
    if (!ret)
      TEST_FAILED("Failed to find farthest point for 1,1");
    if (*ret != Point(-1, -1))
      TEST_FAILED("Farthest point to 1,1 should be -1,-1, not " + ret->str());
  }

  {
    auto ret = ntree.farthest(Point(0.25, 0.1));
    if (!ret)
      TEST_FAILED("Failed to find farthest point for 0.25,0.1");
    if (*ret != Point(-1, -1))
      TEST_FAILED("Farthest point to 0.25,0.1 should be -1,-1, not " + ret->str());
  }

  {
    auto ret = ntree.farthest(Point(1.25, -1.1));
    if (!ret)
      TEST_FAILED("Failed to find farthest point for 1.25,-1.1");
    if (*ret != Point(-1, 1))
      TEST_FAILED("Farthest point to 1.25,-1.1 should be -1,1, not " + ret->str());
  }

  TEST_PASSED();
}

// ----------------------------------------------------------------------

void nearest_many()
{
  std::multimap<double, Point> points;

  points = ntree.nearestones(Point(1, 0), 0.5);
  if (points.size() != 1)
    TEST_FAILED("Failed to find 1 nearest points for 1,0 with radius 0.5");

  points = ntree.nearestones(Point(1, 0), 1.3);
  if (points.size() != 4)
    TEST_FAILED("Failed to find 4 nearest points for 1,0 with radius 1.3");

  points = ntree.nearestones(Point(1, 0), 1.5);
  if (points.size() != 6)
    TEST_FAILED("Failed to find 6 nearest points for 1,0 with radius 1.5");

  points = ntree.nearestones(Point(1, 0), 2.2);
  if (points.size() != 7)
    TEST_FAILED("Failed to find 7 nearest points for 1,0 with radius 2.2");

  points = ntree.nearestones(Point(1, 0), 2.3);
  if (points.size() != 9)
    TEST_FAILED("Failed to find 9 nearest points for 1,0 with radius 2.3");

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
    TEST(nearest_distancelimit);
    TEST(farthest);
    TEST(nearest_many);
  }
};

}  // namespace NearTreeTest

//! The main program
int main(void)
{
  prepare();

  using namespace std;
  cout << endl << "NearTree" << endl << "========" << endl;
  NearTreeTest::tests t;
  return t.run();
}

// ======================================================================
