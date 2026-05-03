// ======================================================================
/*!
 * \file NFmiRadarStation.h
 * \brief Interface of class NFmiRadarStation
 */
// ======================================================================

#pragma once

#include "NFmiStation.h"

//! Undocumented
class NFmiRadarStation : public NFmiStation
{
 public:
  virtual ~NFmiRadarStation();

  NFmiRadarStation();

  NFmiRadarStation(const NFmiRadarStation& theStation);

  NFmiRadarStation(NFmiStation& station,
                   unsigned long resolution,
                   unsigned long theXNumber,
                   unsigned long theYNumber);

  NFmiRadarStation& operator=(const NFmiRadarStation& NFmiRadarStation);
  virtual bool IsEqual(const NFmiSortable& theLocation) const;

  unsigned long Resolution() const;
  unsigned long XNumber() const;
  unsigned long YNumber() const;

  void Resolution(unsigned long theResolution);
  void XNumber(unsigned long theXNumber);
  void YNumber(unsigned long theYNumber);

  virtual NFmiLocation* Clone() const;
  virtual unsigned long ClassId() const;

  virtual std::ostream& Write(std::ostream& file) const;
  virtual std::istream& Read(std::istream& file);

 private:
  unsigned long itsResolution;  //!< res. of radar grid
  unsigned long itsXNumber;     //!< X-Num. of radar grid
  unsigned long itsYNumber;     //!< Y-Num. of radar grid

};  // class NFmiRadarStation

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiRadarStation::Resolution() const
{
  return itsResolution;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiRadarStation::XNumber() const
{
  return itsXNumber;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiRadarStation::YNumber() const
{
  return itsYNumber;
}
// ----------------------------------------------------------------------
/*!
 * \param theResolution Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiRadarStation::Resolution(unsigned long theResolution)
{
  itsResolution = theResolution;
}

// ----------------------------------------------------------------------
/*!
 * \param theXNumber Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiRadarStation::XNumber(unsigned long theXNumber)
{
  itsXNumber = theXNumber;
}
// ----------------------------------------------------------------------
/*!
 * \param theYNumber Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiRadarStation::YNumber(unsigned long theYNumber)
{
  itsYNumber = theYNumber;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 * \todo Should return auto_ptr
 */
// ----------------------------------------------------------------------

inline NFmiLocation* NFmiRadarStation::Clone() const
{
  return new NFmiRadarStation(*this);
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiRadarStation::ClassId() const
{
  return kNFmiRadarStation;
}

// ======================================================================
