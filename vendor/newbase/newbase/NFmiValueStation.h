// ======================================================================
/*!
 * \file NFmiValueStation.h
 * \brief Interface of class NFmiValueStation
 */
// ======================================================================
/*!
 * \class NFmiValueStation
 *
 * Undocumented
 *
 */
// ======================================================================

#pragma once

#include "NFmiStation.h"

//! Undocumented
class NFmiValueStation : public NFmiStation
{
 public:
  virtual ~NFmiValueStation();
  NFmiValueStation();
  NFmiValueStation(const NFmiValueStation &theValueStation);

  NFmiValueStation(double theValue,
                   unsigned long theIdent,
                   const NFmiString &theName = "undefined",
                   double theLongitude = kFloatMissing,
                   double theLatitude = kFloatMissing);

  virtual bool IsEqual(const NFmiSortable &theLocation) const;

  double GetValue() const;
  virtual NFmiLocation *Clone() const;

 private:
  double itsValue;

};  // class NFmiValueStation

// ----------------------------------------------------------------------
/*!
 * Destructor
 */
// ----------------------------------------------------------------------

inline NFmiValueStation::~NFmiValueStation() {}
// ----------------------------------------------------------------------
/*!
 * Void constructor
 */
// ----------------------------------------------------------------------

inline NFmiValueStation::NFmiValueStation() : NFmiStation(0), itsValue(kFloatMissing) {}
// ----------------------------------------------------------------------
/*!
 * Copy constructor
 *
 * \param theValueStation The object being copied
 */
// ----------------------------------------------------------------------

inline NFmiValueStation::NFmiValueStation(const NFmiValueStation &theValueStation)
    : NFmiStation(theValueStation.GetIdent(),
                  theValueStation.GetName(),
                  theValueStation.GetLongitude(),
                  theValueStation.GetLatitude()),
      itsValue(theValueStation.itsValue)
{
}

// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theValue Undocumented
 * \param theIdent Undocumented
 * \param theName Undocumented
 * \param theLongitude Undocumented
 * \param theLatitude Undocumented
 */
// ----------------------------------------------------------------------

inline NFmiValueStation::NFmiValueStation(double theValue,
                                          unsigned long theIdent,
                                          const NFmiString &theName,
                                          double theLongitude,
                                          double theLatitude)
    : NFmiStation(theIdent, theName, theLongitude, theLatitude), itsValue(theValue)
{
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline NFmiLocation *NFmiValueStation::Clone() const
{
  return new NFmiValueStation(*this);
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline double NFmiValueStation::GetValue() const
{
  return itsValue;
}
// ----------------------------------------------------------------------
/*!
 * \param theLocation Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiValueStation::IsEqual(const NFmiSortable &theLocation) const
{
  return (GetValue() == (static_cast<const NFmiValueStation *>((&theLocation))->GetValue()));
}

// ======================================================================
