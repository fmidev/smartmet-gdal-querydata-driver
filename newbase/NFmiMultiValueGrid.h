// ======================================================================
/*!
 * \file NFmiMultiValueGrid.h
 * \brief Interface of class NFmiMultiValueGrid
 */
// ======================================================================

#pragma once

#include "NFmiGrid.h"
#include "NFmiQueryInfo.h"

// infolta iteraattori-ominaisuudet

//! Undocumented

class NFmiMultiValuedGrid : public NFmiGrid, private NFmiQueryInfo
{
 public:
  virtual ~NFmiMultiValuedGrid();

  NFmiMultiValuedGrid();
  NFmiMultiValuedGrid(const NFmiGrid& theGrid,
                      const NFmiParamDescriptor& theParamDescriptor,
                      const NFmiTimeDescriptor& theTimeDescriptor,
                      const NFmiVPlaceDescriptor& theVPlaceDescriptor);

  bool FirstValue();  // Resets Param,Time and Level but not location

  // Ambiquos methods inhereted primarily from NFmiGrid

  bool First();                // resets Grid
  unsigned long Size() const;  // Grid Size
  NFmiArea* Area() const;      // GridArea

  float FloatValue(long dx = 0, long dy = 0);
  using NFmiQueryInfo::FloatValue;
  bool FloatValue(float data);
  bool InterpolateToLatLonPoint(double newLon, double newLat, double& theValue);

 private:
  NFmiMultiValuedGrid(const NFmiMultiValuedGrid& theGrid);
  NFmiMultiValuedGrid& operator=(const NFmiMultiValuedGrid& theGrid);

  void UpDate();
  bool Save();
  bool Edit();

  using NFmiGrid::Init;
  void Init();
  virtual bool Init(NFmiDataPool* theData) { return NFmiGrid::Init(theData); }
  virtual bool Init(NFmiGridBase* theData) { return NFmiGridBase::Init(theData); }
  NFmiDataPool* itsMultiData;
  unsigned long itsCurrentDataStatus;

 protected:
};  // class NFmiMultiValuedGrid

// ----------------------------------------------------------------------
/*!
 * Void constructor
 */
// ----------------------------------------------------------------------

inline NFmiMultiValuedGrid::NFmiMultiValuedGrid() : itsMultiData(0), itsCurrentDataStatus() {}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiMultiValuedGrid::First()
{
  return NFmiGrid::First();
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiMultiValuedGrid::Size() const
{
  return NFmiGrid::Size();
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline NFmiArea* NFmiMultiValuedGrid::Area() const
{
  return NFmiGrid::Area();
}

// ======================================================================
