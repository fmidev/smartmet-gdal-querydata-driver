// ======================================================================
/*!
 * \file NFmiMultiPolygonAreaMask.h
 * \brief Interface of class NFmiMultiPolygonAreaMask
 */
// ======================================================================

#pragma once

#include "NFmiAreaMaskImpl.h"
#include <vector>

class NFmiSvgPath;
class NFmiBitmapAreaMask;
class NFmiGrid;

//! Undocumented
class NFmiMultiPolygonAreaMask : public NFmiAreaMaskImpl
{
 public:
  ~NFmiMultiPolygonAreaMask();

  NFmiMultiPolygonAreaMask();

  NFmiMultiPolygonAreaMask(const NFmiCalculationCondition &theOperation,
                           Type theMaskType,
                           NFmiInfoData::Type theDataType,
                           BinaryOperator thePostBinaryOperator);

  NFmiBitmapAreaMask *CreateBitmapAreaMask(const NFmiGrid &theGrid);

  //@{ \name Kirjoitus- ja luku-operaatiot
  std::ostream &Write(std::ostream &file) const;
  std::istream &Read(std::istream &file);
  //@}

 protected:
  double CalcValueFromLocation(const NFmiPoint &theLatLon) const;
  const NFmiString MakeSubMaskString() const;

 private:
  void Clear();

  std::vector<NFmiSvgPath *> itsAreaPolygons;  //!< Tässä on erilliset alueet svg-polkuina.
  std::vector<double> itsAreaPolygonValues;    //!< Tässä on kunkin alueen maski arvo.
  bool fInitialized;                           //! Set when something has been read from a file

};  // class NFmiMultiPolygonAreaMask

// ----------------------------------------------------------------------
/*!
 * Global output operator from NFmiMultiPolygonAreaMask class
 *
 * \param os The output stream to write to
 * \param item The object to write
 * \return The output stream written to
 */
// ----------------------------------------------------------------------

inline std::ostream &operator<<(std::ostream &os, const NFmiMultiPolygonAreaMask &item)
{
  return item.Write(os);
}

// ----------------------------------------------------------------------
/*!
 * Global input operator from NFmiMultiPolygonAreaMask class
 *
 * \param is The input stream to read from
 * \param item The object to hold the read data
 * \return The input stream read from
 */
// ----------------------------------------------------------------------

inline std::istream &operator>>(std::istream &is, NFmiMultiPolygonAreaMask &item)
{
  return item.Read(is);
}

// ======================================================================
