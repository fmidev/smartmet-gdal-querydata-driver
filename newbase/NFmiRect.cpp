// ======================================================================
/*!
 * \file NFmiRect.cpp
 * \brief Implementation of class NFmiRect
 */
// ======================================================================
/*!
 * \class NFmiRect
 *
 * Undocumented
 *
 */
// ======================================================================

#include "NFmiRect.h"
#include <macgyver/Hash.h>
#include <macgyver/Exception.h>
#include <algorithm>
#include <fstream>

// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param firstCorner Undocumented
 * \param oppositeCorner Undocumented
 */
// ----------------------------------------------------------------------

NFmiRect::NFmiRect(const NFmiPoint &firstCorner, const NFmiPoint &oppositeCorner)
    : itsPlace(std::min(firstCorner.X(), oppositeCorner.X()),
               std::min(firstCorner.Y(), oppositeCorner.Y())),
      itsSize(NFmiPoint(std::max(firstCorner.X(), oppositeCorner.X()),
                        std::max(firstCorner.Y(), oppositeCorner.Y())) -
              itsPlace)
{
}

// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param left Undocumented
 * \param top Undocumented
 * \param right Undocumented
 * \param bottom Undocumented
 */
// ----------------------------------------------------------------------

NFmiRect::NFmiRect(double left, double top, double right, double bottom)
    : itsPlace(std::min(left, right), std::min(top, bottom)),
      itsSize(NFmiPoint(std::max(left, right), std::max(top, bottom)) - itsPlace)
{
}

// ----------------------------------------------------------------------
/*!
 * \param theValue Undocumented
 */
// ----------------------------------------------------------------------

void NFmiRect::Inflate(double theValue)
{
  try
  {
    itsSize += NFmiPoint(2 * theValue, 2 * theValue);
    itsPlace -= NFmiPoint(theValue, theValue);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theXValue Undocumented
 * \param theYValue Undocumented
 */
// ----------------------------------------------------------------------

void NFmiRect::Inflate(double theXValue, double theYValue)
{
  try
  {
    itsSize += NFmiPoint(2 * theXValue, 2 * theYValue);
    itsPlace -= NFmiPoint(theXValue, theYValue);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theValue Undocumented
 */
// ----------------------------------------------------------------------

void NFmiRect::Inflate(const NFmiPoint &theValue)
{
  try
  {
    NFmiPoint theSizingValue(theValue);
    theSizingValue += theValue;
    itsSize += theSizingValue;
    itsPlace -= theValue;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param thePoint Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const NFmiPoint NFmiRect::NearestCorner(const NFmiPoint &thePoint) const
{
  try
  {
    double dLeftTop = TopLeft().Distance(thePoint);
    double dRightTop = TopRight().Distance(thePoint);
    double dLeftBottom = BottomLeft().Distance(thePoint);
    double dRightBottom = BottomRight().Distance(thePoint);

    if (dLeftTop <= dRightTop)
    {
      if (dLeftTop <= dLeftBottom)
        return TopLeft();
      else
        return BottomLeft();
    }
    else
    {
      if (dRightTop <= dRightBottom)
        return TopRight();
      else
        return BottomRight();
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theRatioXperY Undocumented
 * \param fKeepX Undocumented
 * \param theDirection Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

// Säätää rectin aspect ration halutuksi. fKeepX ollessa tosi ei muuteta
// x:n arvoja. Korjaukset tehdään halutun nurkan ('theDirection') suhteen
// eli valittu nurkka "pysyy paikallaan". Jos muutos on mahdoton (esim.
// aspectratio=0), palauttaa funktio false:n muuten true:n.

bool NFmiRect::AdjustAspectRatio(double theRatioXperY, bool fKeepX, FmiDirection theDirection)
{
  try
  {
    if (theRatioXperY == 0.)
      return false;

    NFmiRect originalRect(itsPlace, itsPlace + itsSize);  // 9.9.99/EL

    if (fKeepX)
      Size(NFmiPoint(Width(), Width() / theRatioXperY));
    else
      Size(NFmiPoint(theRatioXperY * Height(), Height()));

    double newWidth = Width();
    double newHeight = Height();

    // Origona on nurkka 'kTopLeft' missä (X,Y) = TopLeft()
    // X kasvaa "vasemmalta oikealle"
    // Y kasvaa "ylhäältä alas"

    switch (theDirection)
    {
      case kTopLeft:
        return true;  // Nurkka on origossa - Place() pysyy paikallaan

      case kTopRight:
        Place(NFmiPoint(originalRect.TopRight().X() - newWidth, TopRight().Y()));
        return true;

      case kBottomLeft:
        Place(NFmiPoint(originalRect.BottomLeft().X(), originalRect.BottomLeft().Y() - newHeight));
        return true;

      case kBottomRight:
        Place(NFmiPoint(originalRect.BottomRight().X() - newWidth,
                        originalRect.BottomRight().Y() - newHeight));
        return true;

      case kCenter:
        Center(originalRect.Center());
        return true;

      default:
        return false;
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Projisoi annetun maailma koordinaatin siten että jos piste on rectin
 * sisällä, palautettu rect saa arvon 0,0 - 1,1 sen mukaan miten se rectin
 * sisään osui. Origo on top-left ja 1,1 on bottom-right kulma. Jos se on
 * ruudun ulkona, tulee mukaan mahdolliset negatiiviset ja 1:stä suuremmat arvot.
 *
 * \param thePoint The relative world point that is projected to rect dimensions.
 * \return The given point in rect's world coordinates.
 */
// ----------------------------------------------------------------------
const NFmiPoint NFmiRect::Project(const NFmiPoint &thePlace) const
{
  try
  {
    NFmiPoint newPoint = thePlace - TopLeft();
    return NFmiPoint(newPoint.X() / Width(), newPoint.Y() / Height());
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Assignment operator
 *
 * \param thePoint The other object being assigned
 * \return The assigned object
 */
// ----------------------------------------------------------------------

NFmiRect &NFmiRect::operator+=(const NFmiPoint &thePoint)
{
  try
  {
    itsPlace += thePoint;
    return *this;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Substraction operator
 *
 * \param thePoint The coordinates being substracted
 * \return The resulting rectangle
 */
// ----------------------------------------------------------------------

NFmiRect &NFmiRect::operator-=(const NFmiPoint &thePoint)
{
  try
  {
    itsPlace -= thePoint;
    return *this;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Addition operator
 *
 * \param theRect The rectangle being added
 * \return The resulting rectangle
 */
// ----------------------------------------------------------------------

NFmiRect &NFmiRect::operator+=(const NFmiRect &theRect)
{
  try
  {
    itsPlace = NFmiPoint(Left() < theRect.Left() ? Left() : theRect.Left(),
                         Top() < theRect.Top() ? Top() : theRect.Top());

    itsSize = NFmiPoint(Right() > theRect.Right() ? Right() : theRect.Right(),
                        Bottom() > theRect.Bottom() ? Bottom() : theRect.Bottom()) -
              itsPlace;
    return *this;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Substraction operator
 *
 * \param theRect The rectangle being substracted
 * \result The resulting rectangle
 */
// ----------------------------------------------------------------------

NFmiRect &NFmiRect::operator-=(const NFmiRect &theRect)
{
  try
  {
    itsPlace = NFmiPoint(Left() > theRect.Left() ? Left() : theRect.Left(),
                         Top() > theRect.Top() ? Top() : theRect.Top());

    itsSize = NFmiPoint(Right() < theRect.Right() ? Right() : theRect.Right(),
                        Bottom() < theRect.Bottom() ? Bottom() : theRect.Bottom()) -
              itsPlace;
    return *this;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param newCenter Undocumented
 */
// ----------------------------------------------------------------------

void NFmiRect::Center(const NFmiPoint &newCenter)
{
  try
  {
    Place(NFmiPoint(newCenter.X() - (Width() / 2.), newCenter.Y() - (Height() / 2.)));
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Palauttaa kahta rectiä ympäröivän rectin.
 * \param theRect Undocumented
 * \result Undocumented
 */
// ----------------------------------------------------------------------

const NFmiRect NFmiRect::SmallestEnclosing(const NFmiRect &theRect) const
{
  try
  {
    double left = Left() < theRect.Left() ? Left() : theRect.Left();
    double top = Top() < theRect.Top() ? Top() : theRect.Top();
    double right = Right() > theRect.Right() ? Right() : theRect.Right();
    double bottom = Bottom() > theRect.Bottom() ? Bottom() : theRect.Bottom();
    return NFmiRect(left, top, right, bottom);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Palauttaa pienimmän rect:in, mikä pitää sisällään this-rect:in ja
 * parametrina annetun rect:in eli leikkaus.
 * \param theRect Undocumented
 * \result Undocumented
 */
// ----------------------------------------------------------------------

const NFmiRect NFmiRect::Intersection(const NFmiRect &theRect) const
{
  try
  {
    double left = Left() > theRect.Left() ? Left() : theRect.Left();
    double top = Top() > theRect.Top() ? Top() : theRect.Top();
    double right = Right() < theRect.Right() ? Right() : theRect.Right();
    double bottom = Bottom() < theRect.Bottom() ? Bottom() : theRect.Bottom();
    return NFmiRect(left, top, right, bottom);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Write the rectangle to the given output stream
 *
 * \param file The output stream to write to
 * \result The output stream written to
 */
// ----------------------------------------------------------------------

std::ostream &NFmiRect::Write(std::ostream &file) const
{
  try
  {
    file << itsPlace;
    file << itsSize;
    return file;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Read new rectangle definition from the given input stream
 *
 * \param file The input stream to read from
 * \return The input stream read from
 */
// ----------------------------------------------------------------------

std::istream &NFmiRect::Read(std::istream &file)
{
  try
  {
    file >> itsPlace;
    file >> itsSize;
    return file;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Addition of a point and a rectangle
 *
 * \param leftPoint Undocumented
 * \param rightRect Undocumented
 * \return Undocumemted
 */
// ----------------------------------------------------------------------

NFmiRect operator+(const NFmiPoint &leftPoint, const NFmiRect &rightRect)
{
  try
  {
    return NFmiRect(
        NFmiPoint(leftPoint.X() + rightRect.Left(), leftPoint.Y() + rightRect.Top()),
        NFmiPoint(leftPoint.X() + rightRect.Right(), leftPoint.Y() + rightRect.Bottom()));
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Substraction of a point and a rectangle
 *
 * \param leftPoint Undocumented
 * \param rightRect Undocumented
 * \return Undocumemted
 */
// ----------------------------------------------------------------------

NFmiRect operator-(const NFmiPoint &leftPoint, const NFmiRect &rightRect)
{
  try
  {
    return NFmiRect(
        NFmiPoint(leftPoint.X() - rightRect.Left(), leftPoint.Y() - rightRect.Top()),
        NFmiPoint(leftPoint.X() - rightRect.Right(), leftPoint.Y() - rightRect.Bottom()));
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Addition of a point and a rectangle
 *
 * \param leftRect Undocumented
 * \param rightPoint Undocumented
 * \result Undocumented
 */
// ----------------------------------------------------------------------

NFmiRect operator+(const NFmiRect &leftRect, const NFmiPoint &rightPoint)
{
  try
  {
    return NFmiRect(
        NFmiPoint(rightPoint.X() + leftRect.Left(), rightPoint.Y() + leftRect.Top()),
        NFmiPoint(rightPoint.X() + leftRect.Right(), rightPoint.Y() + leftRect.Bottom()));
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Substraction of a rectangle and a point
 *
 * \param leftRect Undocumented
 * \param rightPoint Undocumented
 * \result Undocumented
 */
// ----------------------------------------------------------------------

NFmiRect operator-(const NFmiRect &leftRect, const NFmiPoint &rightPoint)
{
  try
  {
    return NFmiRect(
        NFmiPoint(rightPoint.X() - leftRect.Left(), rightPoint.Y() - leftRect.Top()),
        NFmiPoint(rightPoint.X() - leftRect.Right(), rightPoint.Y() - leftRect.Bottom()));
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

bool NFmiRect::Intersect(const NFmiRect &theRect) const
{
  try
  {
    if (this->Left() < theRect.Right() && this->Right() > theRect.Left() &&
        this->Top() < theRect.Bottom() && this->Bottom() > theRect.Top())
      return true;
    else
      return false;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Return hash value for the rectangle
 */
// ----------------------------------------------------------------------

std::size_t NFmiRect::HashValue() const
{
  try
  {
    std::size_t hash = itsPlace.HashValue();
    Fmi::hash_combine(hash, itsSize.HashValue());
    return hash;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ======================================================================
