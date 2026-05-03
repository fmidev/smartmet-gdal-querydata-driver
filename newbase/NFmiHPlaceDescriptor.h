// ======================================================================
/*!
 * \file NFmiHPlaceDescriptor.h
 * \brief Interface of class NFmiHPlaceDescriptor
 */
// ======================================================================

#pragma once

#include "NFmiDataDescriptor.h"
#include "NFmiDataMatrix.h"
#include "NFmiGlobals.h"
#include "NFmiLocation.h"
#include "NFmiPoint.h"
#include "NFmiSaveBaseFactory.h"
#include <algorithm>

class NFmiLocationBag;
class NFmiArea;
class NFmiGrid;
class NFmiLocation;

namespace Fmi
{
class CoordinateMatrix;
class SpatialReference;
}  // namespace Fmi

//! Undocumented
class NFmiHPlaceDescriptor : public NFmiDataDescriptor
{
 public:
  virtual ~NFmiHPlaceDescriptor();
  NFmiHPlaceDescriptor();
  NFmiHPlaceDescriptor(const NFmiHPlaceDescriptor &theHPlaceDescriptor);

  NFmiHPlaceDescriptor(const NFmiLocationBag &theLocationBag,
                       FmiStationType theSelectedType = kWMO,
                       unsigned long theMaxNumberOfSources = 0);

  NFmiHPlaceDescriptor(const NFmiArea &theArea,
                       FmiStationType theSelectedType = kWMO,
                       unsigned long theMaxNumberOfSources = 0);

  NFmiHPlaceDescriptor(const NFmiGrid &theGrid,
                       FmiStationType theSelectedType = kLatLon,
                       unsigned long theMaxNumberOfSources = 0);

  NFmiHPlaceDescriptor(const NFmiLocationBag &theLocationBag,
                       const NFmiArea &theArea,
                       FmiStationType theSelectedType = kAll,
                       unsigned long theMaxNumberOfSources = 0);

  NFmiHPlaceDescriptor(const NFmiArea &theArea,
                       const NFmiGrid &theGrid,
                       FmiStationType theSelectedType = kLatLon,
                       unsigned long theMaxNumberOfSources = 0);

  bool IsLocation() const;
  bool IsArea() const;
  bool IsGrid() const;

  const NFmiArea *Area() const;
  const NFmiGrid *Grid() const;
  NFmiGrid *NonConstGrid() const;
  const NFmiLocation *Location() const;
  const NFmiLocation *LocationWithIndex(unsigned long theIndex) const;

  NFmiPoint WorldXY(
      unsigned long index) const;  // actual metric coordinate, or latlon for point data

  const Fmi::SpatialReference &SpatialReference() const;
  Fmi::CoordinateMatrix CoordinateMatrix(bool wrapped) const;
  bool NeedsGlobeWrap() const;

  NFmiPoint LatLon() const;
  NFmiPoint LatLon(unsigned long theIndex) const;
  NFmiPoint RelativePoint() const;
  NFmiPoint RelativePoint(unsigned long theIndex) const;
  NFmiPoint GridPoint() const;

  bool NearestLocation(const NFmiLocation &theLocation,
                       double theMaxDistance = kFloatMissing * 1000.);
  bool NearestLocation(const NFmiLocation &theLocation,
                       const NFmiArea *theArea,
                       double theMaxDistance = kFloatMissing * 1000.);
  bool NearestPoint(const NFmiPoint &theLatLonPoint);

  //! Hakee listan paikkaindeksi/etäisyys metreinä pareja. Listaan haetaan annettua paikkaa lähimmat
  //! datapisteet.
  const std::vector<std::pair<int, double> > NearestLocations(
      const NFmiLocation &theLocation,
      int theMaxWantedLocations,
      double theMaxDistance = kFloatMissing) const;

  bool MoveInGrid(long xSteps, long ySteps);

  FmiInterpolationMethod InterpolationMethod() const;

  void LocationList(const NFmiLocationBag &theLocationBag);
  bool Location(const NFmiLocation &theLocation);
  bool Location(long theIdent);
  bool Location(const NFmiString &theLocationName);
  bool Location(const NFmiPoint &theLonLatPoint, NFmiPoint *theGridPoint = 0);

  void Reset();
  bool First();
  bool Next();
  bool Previous();

  void CreateLatLonCache(std::vector<NFmiPoint> &v);

  virtual unsigned long Index() const;
  bool Index(unsigned long theIndex);
  virtual unsigned long Size() const;

  virtual FmiStationType SelectedType() const;
  virtual void SelectedType(FmiStationType thelocationType);

  virtual unsigned long MaxNumberOfSources() const;
  virtual void MaxNumberOfSources(unsigned long theMaxNumberOfSources);
  virtual bool IsMaxNumberOfSources() const;

  virtual bool IsActive() const;
  virtual bool SetActivity(bool);
  virtual bool NextActive();

  virtual const NFmiHPlaceDescriptor Combine(const NFmiHPlaceDescriptor &theCombine);

  virtual std::ostream &Write(std::ostream &file) const;
  virtual std::istream &Read(std::istream &file);

  virtual unsigned long ClassId() const;
  virtual const char *ClassName() const;

  NFmiHPlaceDescriptor &operator=(const NFmiHPlaceDescriptor &theHPlaceDescriptor);
  bool operator==(const NFmiHPlaceDescriptor &theHPlaceDescriptor) const;

  bool IsInside(const NFmiPoint &theLatLon, double theRadius) const;

  std::size_t HashValue() const;

 protected:
  void Destroy();

 private:
  NFmiLocationBag *itsLocationBag;

  NFmiArea *itsArea;
  FmiStationType itsSelectedType;
  unsigned long itsMaxNumberOfSources;

  NFmiGrid *itsGrid;

  bool *itsActivity;

};  // class NFmiHPlaceDescriptor

// ----------------------------------------------------------------------
/*!
 * Destructor
 */
// ----------------------------------------------------------------------

inline NFmiHPlaceDescriptor::~NFmiHPlaceDescriptor()
{
  Destroy();
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiHPlaceDescriptor::IsLocation() const
{
  return (itsLocationBag != 0);
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiHPlaceDescriptor::IsArea() const
{
  return (itsArea != 0);
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiHPlaceDescriptor::IsGrid() const
{
  return (itsGrid != 0);
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const NFmiGrid *NFmiHPlaceDescriptor::Grid() const
{
  return itsGrid;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline NFmiGrid *NFmiHPlaceDescriptor::NonConstGrid() const
{
  return itsGrid;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline FmiStationType NFmiHPlaceDescriptor::SelectedType() const
{
  return itsSelectedType;
}
// ----------------------------------------------------------------------
/*!
 * \param theStationType Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiHPlaceDescriptor::SelectedType(FmiStationType theStationType)
{
  itsSelectedType = theStationType;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiHPlaceDescriptor::MaxNumberOfSources() const
{
  return itsMaxNumberOfSources;
}

// ----------------------------------------------------------------------
/*!
 * \param theMaxNumberOfSources Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiHPlaceDescriptor::MaxNumberOfSources(unsigned long theMaxNumberOfSources)
{
  itsMaxNumberOfSources = theMaxNumberOfSources;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiHPlaceDescriptor::IsMaxNumberOfSources() const
{
  return itsMaxNumberOfSources ? true : false;
}

// ----------------------------------------------------------------------
/*!
 * Output operator for class NFmiHPlaceDescriptor
 *
 * \param file The output stream to write to
 * \param ob The object to write
 * \return The output stream written to
 */
// ----------------------------------------------------------------------

inline std::ostream &operator<<(std::ostream &file, const NFmiHPlaceDescriptor &ob)
{
  return ob.Write(file);
}

// ----------------------------------------------------------------------
/*!
 * Input operator for class NFmiHPlaceDescriptor
 *
 * \param file The input stream to read from
 * \param ob The object into which to read the new contents
 * \return The input stream read from
 */
// ----------------------------------------------------------------------

inline std::istream &operator>>(std::istream &file, NFmiHPlaceDescriptor &ob)
{
  return ob.Read(file);
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiHPlaceDescriptor::ClassId() const
{
  return kNFmiHPlaceDescriptor;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const char *NFmiHPlaceDescriptor::ClassName() const
{
  return "NFmiHPlaceDescriptor";
}

// ======================================================================
