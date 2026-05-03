// ======================================================================
/*!
 * \file NFmiStation.h
 * \brief Interface of class NFmiStation
 */
// ======================================================================

#pragma once

#include "NFmiLocation.h"

//! Undocumented
class NFmiStation : public NFmiLocation
{
 public:
  virtual ~NFmiStation();

  NFmiStation();

  NFmiStation(const NFmiStation& theStation);

  NFmiStation(long theIdent,
              const NFmiString& theName = "undefined",
              double theLongitude = static_cast<double>(kFloatMissing),
              double theLatitude = static_cast<double>(kFloatMissing),
              double theMaxDistance = static_cast<double>(kFloatMissing),
              FmiStationType theIdentType = kWMO);

  NFmiStation(NFmiIndividual theIdent,
              NFmiLocation theLocation,
              double theMaxDistance = static_cast<double>(kFloatMissing),
              FmiStationType theIdentType = kWMO);

  NFmiStation& operator=(const NFmiStation& theStation);

  virtual bool IsEqual(const NFmiSortable& theLocation) const;
  virtual bool IsLessThan(const NFmiSortable& theLocation) const;
  virtual bool IsEqualName(const NFmiStation& theStation) const;

  virtual NFmiLocation* Clone() const;
  virtual unsigned long ClassId() const { return kNFmiStation; }
  virtual double MaxDistance() const { return itsMaxDistance; }
  virtual void MaxDistance(double theMaxDistance) { itsMaxDistance = theMaxDistance; }
  virtual FmiStationType IdentType() const { return itsIdentType; }
  virtual void IdentType(FmiStationType theIdentType) { itsIdentType = theIdentType; }
  virtual std::ostream& Write(std::ostream& file) const;
  virtual std::istream& Read(std::istream& file);

 protected:
  virtual void SetLocation(const NFmiLocation& theLocations);

  double itsMaxDistance;
  FmiStationType itsIdentType;

 private:
};  // class NFmiStation

// ----------------------------------------------------------------------
/*!
 * Output operator for class NFmiStation
 *
 * \param os The output stream to write to
 * \param item The object to write
 * \return The output stream written to
 */
// ----------------------------------------------------------------------

inline std::ostream& operator<<(std::ostream& os, const NFmiStation& item)
{
  return item.Write(os);
}

// ----------------------------------------------------------------------
/*!
 * Input operator for class NFmiStation
 *
 * \param is The input stream to read from
 * \param item The object into which to read the new contents
 * \return The input stream read from
 */
// ----------------------------------------------------------------------

inline std::istream& operator>>(std::istream& is, NFmiStation& item)
{
  return item.Read(is);
}

// ======================================================================
