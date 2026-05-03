// ======================================================================
/*!
 * \file NFmiLocation.h
 * \brief Interface of class NFmiLocation
 */
// ======================================================================

#pragma once

#include "NFmiIndividual.h"
#include "NFmiPoint.h"
#include "NFmiSaveBaseFactory.h"
#include <algorithm>

class NFmiTime;
class NFmiMetTime;

//! Undocumented

class NFmiLocation : public NFmiIndividual
{
 public:
  virtual ~NFmiLocation() {}
  NFmiLocation() : NFmiIndividual(0, "undefined"), itsLatlon(NFmiPoint::gMissingLatlon) {}
  NFmiLocation(double theLongitude, double theLatitude)
      : NFmiIndividual(0, "undefined"), itsLatlon(theLongitude, theLatitude)
  {
  }

  NFmiLocation(const NFmiPoint& theLonLatPoint)
      : NFmiIndividual(0, "undefined"), itsLatlon(theLonLatPoint)
  {
  }

  NFmiLocation(const NFmiLocation& theLocation)
      : NFmiIndividual(theLocation.GetIdent(), theLocation.GetName()),
        itsLatlon(theLocation.itsLatlon)
  {
  }

  bool operator==(const NFmiLocation& theLocation) const;
  bool operator<(const NFmiLocation& theLocation) const;

  void SetLatitude(double theLatitude) { itsLatlon.Y(theLatitude); }
  void SetLongitude(double theLongitude) { itsLatlon.X(theLongitude); }
  double GetLatitude() const { return itsLatlon.Y(); }
  double GetLongitude() const { return itsLatlon.X(); }
  const NFmiPoint& GetLocation() const { return itsLatlon; }
  void SetLocation(const NFmiPoint& newLocation) { itsLatlon = newLocation; }
  // 9.8.04/EL -->
  void SetLocation(double theAzimuthInDegrees, double theDistanceInMeters, bool usePacificView);
  const NFmiLocation GetLocation(double theAzimuthInDegrees,
                                 double theDistanceInMeters,
                                 bool usePacificView) const;
  // 9.8.04/EL <--

  double Distance(const NFmiLocation& theLocation) const;
  double Distance(const NFmiPoint& theLonLat) const;
  double Direction(const NFmiPoint& theLatLon) const;
  double Direction(const NFmiLocation& theLocation) const
  {
    return Direction(theLocation.GetLocation());
  }

  double ElevationAngle(const NFmiMetTime& theTime);
  double ElevationAngleFromSolarTime(const NFmiTime& theSolarTime);
  double AtzimuthAngle(const NFmiMetTime& theTime);
  double DeclinationAngle(const NFmiMetTime& theTime);
  NFmiTime LocalSolarTime(const NFmiMetTime& theTime);
  NFmiMetTime UtcTimeFromSolarTime(const NFmiTime& theSolarTime);
  NFmiMetTime TimeOfSunrise(const NFmiMetTime& theTime, bool& isCurrentDay);
  NFmiMetTime TimeOfSunset(const NFmiMetTime& theTime, bool& isCurrentDay);
  NFmiMetTime NextTimeOfSunset(const NFmiMetTime& theTime, bool& isMidNightSun, bool& isPolarNight);
  NFmiMetTime NextTimeOfSunrise(const NFmiMetTime& theTime,
                                bool& isMidNightSun,
                                bool& isPolarNight);
  NFmiMetTime LastTimeOfSunset(const NFmiMetTime& theTime, bool& isMidNightSun, bool& isPolarNight);
  NFmiMetTime LastTimeOfSunrise(const NFmiMetTime& theTime,
                                bool& isMidNightSun,
                                bool& isPolarNight);

  NFmiLocation& operator=(const NFmiLocation& theLocation);
  virtual bool IsEqual(const NFmiSortable& aFmiTest) const;
  virtual bool IsLessThan(const NFmiSortable& aFmiTest) const;

  virtual NFmiLocation* Clone() const;
  virtual unsigned long ClassId() const { return kNFmiLocation; }
  virtual std::ostream& Write(std::ostream& file) const;
  virtual std::istream& Read(std::istream& file);

  std::size_t HashValue() const;

 protected:
  virtual void SetLocation(const NFmiLocation& theLocations);

  explicit NFmiLocation(long theIdent,
                        const NFmiString& theName = "undefined",
                        double theLongitude = kFloatMissing,
                        double theLatitude = kFloatMissing)
      : NFmiIndividual(theIdent, theName), itsLatlon(theLongitude, theLatitude)
  {
  }

  NFmiLocation ComputeLocation(double theAzimuthInDegrees,
                               double theDistanceInMeters,
                               bool usePacificView) const;  // 9.8.04/EL

 private:
  double CalcDeclinationAngle(const NFmiTime& theSolarTime);
  double CalcAtzimuthAngle(const NFmiTime& theSolarTime);
  void CalcSunriseOrSunsetTime(NFmiTime& theSolarTime);

 protected:
  NFmiPoint itsLatlon;

};  // class NFmiLocation

// ----------------------------------------------------------------------
/*!
 * \param theLonLat Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

// HUOM!! Pitäisi tehdä yllä tehdyt korjaukset varmaan tähänkin metodiin/Marko

inline double NFmiLocation::Distance(const NFmiPoint& theLonLat) const
{
  using namespace std;
  // Returns metric distance between two points on a spherical globe.

  double lo1 = FmiRad(GetLongitude());
  double la1 = FmiRad(GetLatitude());

  double lo2 = FmiRad(theLonLat.X());
  double la2 = FmiRad(theLonLat.Y());

  return kRearth * acos(cos(la2 - la1) - (1. - cos(lo2 - lo1)) * cos(la1) * cos(la2));
}

// ======================================================================
