// ======================================================================
/*!
 * \file NFmiBitmapAreaMask.cpp
 * \brief Implementation of class NFmiBitmapAreaMask
 */
// ======================================================================

#include "NFmiBitmapAreaMask.h"
#include "NFmiArea.h"
#include "NFmiFastQueryInfo.h"
#include "NFmiGrid.h"
#include <macgyver/Exception.h>

// ----------------------------------------------------------------------
/*!
 * Destructor
 */
// ----------------------------------------------------------------------

NFmiBitmapAreaMask::~NFmiBitmapAreaMask()
{
  try
  {
    Destroy();
  }
  catch (...)
  {
    Fmi::Exception exception(BCP, "Destructor failed", nullptr);
    exception.printError();
  }
}

// ----------------------------------------------------------------------
/*!
 * Void constructor
 */
// ----------------------------------------------------------------------

NFmiBitmapAreaMask::NFmiBitmapAreaMask()
    : NFmiAreaMaskImpl(),
      itsBitmask(),
      itsArea(nullptr),
      itsGridXSize(0),
      itsGridYSize(0),
      itsDataIdent(nullptr),
      itsLevel(nullptr)
{
}

// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theXSize Undocumented
 * \param theYSize Undocumented
 * \param theArea Undocumented
 * \param theDataIdent Undocumented
 * \param theLevel Undocumented
 * \param thePostBinaryOperator Undocumented
 */
// ----------------------------------------------------------------------

NFmiBitmapAreaMask::NFmiBitmapAreaMask(int theXSize,
                                       int theYSize,
                                       const NFmiArea* theArea,
                                       const NFmiDataIdent* theDataIdent,
                                       const NFmiLevel* theLevel,
                                       BinaryOperator thePostBinaryOperator)
    : NFmiAreaMaskImpl(
          NFmiCalculationCondition(), kBinary, NFmiInfoData::kStationary, thePostBinaryOperator),
      itsBitmask(theXSize * theYSize, false),
      itsArea(theArea ? theArea->Clone() : nullptr),
      itsGridXSize(theXSize),
      itsGridYSize(theYSize),
      itsDataIdent(theDataIdent ? new NFmiDataIdent(*theDataIdent) : nullptr),
      itsLevel(theLevel ? new NFmiLevel(*theLevel) : nullptr)
{
}

// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theGrid Undocumented
 * \param theDataIdent Undocumented
 * \param theLevel Undocumented
 * \param thePostBinaryOperator Undocumented
 */
// ----------------------------------------------------------------------

NFmiBitmapAreaMask::NFmiBitmapAreaMask(const NFmiGrid& theGrid,
                                       const NFmiDataIdent* theDataIdent,
                                       const NFmiLevel* theLevel,
                                       BinaryOperator thePostBinaryOperator)
    : NFmiAreaMaskImpl(
          NFmiCalculationCondition(), kBinary, NFmiInfoData::kStationary, thePostBinaryOperator),
      itsBitmask(theGrid.XNumber() * theGrid.YNumber(), false),
      itsArea(theGrid.Area()->Clone()),
      itsGridXSize(theGrid.XNumber()),
      itsGridYSize(theGrid.YNumber()),
      itsDataIdent(theDataIdent ? new NFmiDataIdent(*theDataIdent) : nullptr),
      itsLevel(theLevel ? new NFmiLevel(*theLevel) : nullptr)
{
}

// ----------------------------------------------------------------------
/*!
 * Copy constructor
 *
 * \param theMask The other object being copied
 *
 */
// ----------------------------------------------------------------------

NFmiBitmapAreaMask::NFmiBitmapAreaMask(const NFmiBitmapAreaMask& theMask)
    : NFmiAreaMaskImpl(theMask),
      itsBitmask(theMask.itsBitmask),
      itsArea(theMask.itsArea ? theMask.itsArea->Clone() : nullptr),
      itsGridXSize(theMask.itsGridXSize),
      itsGridYSize(theMask.itsGridYSize),
      itsDataIdent(theMask.itsDataIdent ? new NFmiDataIdent(*theMask.itsDataIdent) : nullptr),
      itsLevel(theMask.itsLevel ? new NFmiLevel(*theMask.itsLevel) : nullptr)
{
}

// ----------------------------------------------------------------------
/*!
 * Clone the object
 *
 * \return A new copy of the object
 * \todo Should return an std::shared_ptr
 */
// ----------------------------------------------------------------------

NFmiAreaMask* NFmiBitmapAreaMask::Clone() const
{
  return new NFmiBitmapAreaMask(*this);
}
// ----------------------------------------------------------------------
/*!
 * Internal data destructuction utility
 */
// ----------------------------------------------------------------------

void NFmiBitmapAreaMask::Destroy()
{
  try
  {
    delete itsArea;
    itsArea = nullptr;
    delete itsDataIdent;
    itsDataIdent = nullptr;
    delete itsLevel;
    itsLevel = nullptr;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Initialize the binary mask based on the given info and condition,
 * making the mask the size of the grid in the given info. Note that
 * the info is not stored for later use.
 *
 * \param theInfo The info containing the grid
 * \param theOperation The condition
 */
// ----------------------------------------------------------------------

#ifdef UNIX
// RHEL6 compiler bug forces to omit the reference
void NFmiBitmapAreaMask::Init(std::shared_ptr<NFmiFastQueryInfo> theInfo,
                              const NFmiCalculationCondition& theOperation)
#else
void NFmiBitmapAreaMask::Init(std::shared_ptr<NFmiFastQueryInfo>& theInfo,
                              const NFmiCalculationCondition& theOperation)
#endif
{
  try
  {
    Destroy();
    itsBitmask.resize(theInfo->HPlaceDescriptor().Size(), false);
    itsArea = theInfo->Area() ? theInfo->Area()->Clone() : nullptr;
    itsDataIdent = new NFmiDataIdent(theInfo->Param());
    itsLevel = theInfo->Level() ? new NFmiLevel(*theInfo->Level()) : nullptr;
    if (theInfo->IsGrid())
    {
      itsGridXSize = theInfo->Grid()->XNumber();
      itsGridYSize = theInfo->Grid()->YNumber();
    }
    float value = kFloatMissing;
    for (theInfo->ResetLocation(); theInfo->NextLocation();)
    {
      value = theInfo->FloatValue();
      itsBitmask[theInfo->LocationIndex()] = theOperation.IsMasked(value);
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theLatLon Undocumented
 * \return Undocumented
 *
 * \note If the mask is not enabled, true is returned for all points
 */
// ----------------------------------------------------------------------

bool NFmiBitmapAreaMask::IsMasked(const NFmiPoint& theLatLon) const
{
  try
  {
    if (!fEnabled)
      return true;
    else
      return IsMasked(LatLon2Index(theLatLon));
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * A special case for optimized use. This cannot be use for all classes,
 * but is suitable for example for NFmiInfoAreaMask and NFmiBinaryAreaMask
 * classes.
 *
 * \param theIndex Undocumented
 * \return Undocumented
 *
 * \note If the mask is not enabled, true is returned for all points
 */
// ----------------------------------------------------------------------

bool NFmiBitmapAreaMask::IsMasked(int theIndex) const
{
  try
  {
    if (!fEnabled)
      return true;
    else if (CheckIndex(theIndex))
      return itsBitmask[theIndex];

    return false;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theLatLon Undocumented
 * \return Undocumented
 *
 * \note If the mask is not enabled, true is returned for all points
 */
// ----------------------------------------------------------------------.

// Maskin luonteesta johtuen tämä ei käytä laskuissaan normaaleja suurempi tai pienempi ehtoja,
// vaan jos maski on päällä, arvo on yksi ja muuten nolla.

double NFmiBitmapAreaMask::MaskValue(const NFmiPoint& theLatLon) const
{
  try
  {
    if (!fEnabled)
      return 1;
    else
      return IsMasked(LatLon2Index(theLatLon)) ? 1 : 0;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theLatLon Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

// Maskin luonteesta johtuen tämä ei käytä laskuissaan normaaleja suurempi tai pienempi ehtoja,
// vaan jos maski on päällä, arvo on yksi ja muuten nolla.
// Huom!! tässä ei tarvitse tarkistaa onko enabloitu, koska se tehdään ylempänä.

double NFmiBitmapAreaMask::CalcValueFromLocation(const NFmiPoint& theLatLon) const
{
  try
  {
    return IsMasked(LatLon2Index(theLatLon)) ? 1 : 0;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 * \todo Not implemented yet
 */
// ----------------------------------------------------------------------

const NFmiString NFmiBitmapAreaMask::MakeSubMaskString() const
{
  try
  {
    return NFmiString("NFmiBitmapAreaMask::MakeSubMaskString ei ole toteutettu");
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theIndex Undocumented
 * \param newStatus Undocumented
 */
// ----------------------------------------------------------------------

void NFmiBitmapAreaMask::Mask(int theIndex, bool newStatus)
{
  try
  {
    if (CheckIndex(theIndex))
      itsBitmask[theIndex] = newStatus;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theLatLon Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

int NFmiBitmapAreaMask::LatLon2Index(const NFmiPoint& theLatLon) const
{
  try
  {
    if (itsArea)
    {
      NFmiPoint xyPoint(itsArea->ToXY(theLatLon));  // tämä on 'suhteellisessa' 0,0 - 1,1 maailmassa
      auto x = static_cast<int>(round((itsGridXSize - 1) * itsArea->Width() * xyPoint.X()));
      auto y = static_cast<int>(round((itsGridYSize - 1) * itsArea->Height() * xyPoint.Y()));
      // pitää vielä kääntää
      y = static_cast<int>(round((itsGridYSize - 1) * itsArea->Height() - y));
      int finalIndex = y * itsGridXSize + x;
      return finalIndex;
    }
    return -1;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// HUOM! toimii vain NFmiBinaryMask:in kanssa. Asettaa koko maskin kaikki arvot halutuksi.
void NFmiBitmapAreaMask::SetAll(bool theNewState)
{
  try
  {
    std::vector<bool>(itsBitmask.size(), theNewState).swap(itsBitmask);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ======================================================================
