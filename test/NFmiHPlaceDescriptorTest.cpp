// ======================================================================
/*!
 * \file
 * \brief Regression tests for class NFmiHPlaceDescriptor
 */
// ======================================================================

#include "NFmiHPlaceDescriptor.h"
#include "NFmiQueryData.h"
#include "NFmiStringTools.h"
#include <regression/tframe.h>
#include <fstream>
#include <stdexcept>
#include <string>

using namespace std;

// The test data

NFmiQueryData qd;
NFmiHPlaceDescriptor qi;

//! Protection against conflicts with global functions
namespace NFmiHPlaceDescriptorTest
{
// ----------------------------------------------------------------------

void nearestpoint()
{
  NFmiQueryData qd("data/pistedata.fqd");
  const NFmiHPlaceDescriptor& hdesc = qd.Info()->HPlaceDescriptor();
  NFmiHPlaceDescriptor h(hdesc);

  if (!h.NearestPoint(NFmiPoint(25, 60)))
    TEST_FAILED("Failed to set Harmaja coordinates");
  if (h.Location()->GetName() != NFmiString("Harmaja"))
    TEST_FAILED("Wrong name for coordinates 25,60: " +
                std::string(h.Location()->GetName().CharPtr()));

  TEST_PASSED();
}

// ----------------------------------------------------------------------

void nearestlocation()
{
  NFmiQueryData qd("data/pistedata.fqd");
  const NFmiHPlaceDescriptor& hdesc = qd.Info()->HPlaceDescriptor();
  NFmiHPlaceDescriptor h(hdesc);

  NFmiLocation harmaja(25, 60);
  if (!h.NearestLocation(harmaja))
    TEST_FAILED("Failed to set Harmaja coordinates");
  if (h.Location()->GetName() != NFmiString("Harmaja"))
    TEST_FAILED("Wrong name for coordinates 25,60: " +
                std::string(h.Location()->GetName().CharPtr()));

  if (h.NearestLocation(harmaja, 10 * 1000))
    TEST_FAILED("Should fail to set Harmaja coordinates with 10 km max radius");

  TEST_PASSED();
}

// ----------------------------------------------------------------------

void nearestlocations()
{
  NFmiQueryData qd("data/pistedata.fqd");
  const NFmiHPlaceDescriptor& h = qd.Info()->HPlaceDescriptor();

  NFmiLocation harmaja(25, 60);

  {
    // Search max 10 locations with unlimited radius
    auto res = h.NearestLocations(harmaja, 10);

    if (res.size() != 10)
      TEST_FAILED("Should have found 10 locations since there was no search radius");

    if (res[0].first != 142)
      TEST_FAILED("Should have found Harmaja (id=142) first");
  }

  {
    // Search max 10 locations with limited radius
    auto res = h.NearestLocations(harmaja, 10, 50 * 1000);

    if (res.size() != 8)
      TEST_FAILED("Should have found 8 locations with search radius 50 km")

    if (res[0].first != 142)
      TEST_FAILED("Should have found Harmaja (id=142) first");
  }

  TEST_PASSED();
}

// ----------------------------------------------------------------------

void isinside()
{
  // Test point data first
  {
    NFmiQueryData qd("data/pistedata.fqd");
    const NFmiHPlaceDescriptor& h = qd.Info()->HPlaceDescriptor();

    if (h.IsInside(NFmiPoint(25, 60), 10 * 1000))
      TEST_FAILED("Should not have found Harmaja with a 10 km radius");
    if (!h.IsInside(NFmiPoint(25, 60), 20 * 1000))
      TEST_FAILED("Should have found Harmaja with a 20 km radius");
  }

  // Test grid data

  {
    NFmiQueryData qd("data/hiladata.sqd");

    const NFmiHPlaceDescriptor& h = qd.Info()->HPlaceDescriptor();

    if (!h.IsInside(NFmiPoint(25, 60), 10 * 1000))
      TEST_FAILED("Should have found coordinate 25,60 with a 10 km radius");
    if (h.IsInside(NFmiPoint(0, 0), 10 * 1000))
      TEST_FAILED("Should not have found 0,0 from the grid data");
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
    TEST(nearestpoint);
    TEST(nearestlocation);
    TEST(nearestlocations);
    TEST(isinside);
  }
};

}  // namespace NFmiHPlaceDescriptorTest

//! The main program
int main(void)
{
  using namespace std;
  cout << endl << "NFmiHPlaceDescriptor tester" << endl << "===========================" << endl;
  NFmiHPlaceDescriptorTest::tests t;
  return t.run();
}

// ======================================================================
