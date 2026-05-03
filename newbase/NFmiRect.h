// ======================================================================
/*!
 * \file NFmiRect.h
 * \brief Interface of class NFmiRect
 */
// ======================================================================

#pragma once

#include "NFmiPoint.h"

//! Undocumented
class NFmiRect
{
 public:
  virtual ~NFmiRect() {}
  NFmiRect();
  NFmiRect(const NFmiPoint &theTopLeftCorner, const NFmiPoint &theBottomRightCorner);
  NFmiRect(double Left, double Top, double Right, double Bottom);

  void Set(const NFmiPoint &theTopLeftCorner, const NFmiPoint &theBottomRightCorner);
  void Top(double theValue);
  void Bottom(double theValue);
  void Left(double theValue);
  void Right(double theValue);

  void Inflate(double theXYValue);          // Resize as centered
  void Inflate(double x, double y);         // Resize as centered
  void Inflate(const NFmiPoint &theValue);  // Resize as centered

  double Top() const;
  double Bottom() const;
  double Left() const;
  double Right() const;

  double Height() const;
  double Width() const;
  void Height(double theValue) { itsSize.Y(theValue); }
  void Width(double theValue) { itsSize.X(theValue); }
  const NFmiPoint TopLeft() const;
  const NFmiPoint BottomRight() const;
  const NFmiPoint TopRight() const;
  const NFmiPoint BottomLeft() const;

  const NFmiPoint Corner(FmiDirection cornerIdent) const;

  const NFmiPoint &Size() const;
  const NFmiPoint &Place() const;

  void Place(const NFmiPoint &newPlace);
  void Size(const NFmiPoint &newSize);
  void Center(const NFmiPoint &newCenter);
  const NFmiPoint Center() const;

  bool IsInside(const NFmiPoint &thePoint) const;
  bool IsInside(const NFmiRect &theRect) const;
  bool Intersect(const NFmiRect &theRect) const;
  bool IsEmpty() const;
  const NFmiRect SmallestEnclosing(const NFmiRect &theRect) const;
  const NFmiRect Intersection(const NFmiRect &theRect) const;

  const NFmiPoint NearestCorner(const NFmiPoint &thePoint) const;

  const NFmiRect ToAbs(const NFmiRect &theRect) const;
  const NFmiPoint ToAbs(const NFmiPoint &thePoint) const;

  bool AdjustAspectRatio(double theRatioXperY,
                         bool fKeepX = true,
                         FmiDirection theDirection = kTopLeft);
  const NFmiPoint Project(const NFmiPoint &thePlace) const;

  bool operator==(const NFmiRect &theRect) const;
  bool operator!=(const NFmiRect &theRect) const;

  // NFmiRect & operator=(const NFmiRect & theRect);      // kommentoitu optimoinnin takia

  NFmiRect &operator+=(const NFmiRect &theRect);  // Resize itself
  NFmiRect &operator-=(const NFmiRect &theRect);  // with fixed topleft corner

  NFmiRect &operator+=(const NFmiPoint &thePoint);  // Move itself
  NFmiRect &operator-=(const NFmiPoint &thePoint);  // Move itself

  friend NFmiRect operator+(const NFmiPoint &leftPoint,  // Move
                            const NFmiRect &rightRect);
  friend NFmiRect operator-(const NFmiPoint &leftPoint,  // Move
                            const NFmiRect &rightRect);
  friend NFmiRect operator+(const NFmiRect &leftRect,  // Move
                            const NFmiPoint &rightPoint);
  friend NFmiRect operator-(const NFmiRect &leftRect,  // Move
                            const NFmiPoint &rightPoint);

  std::ostream &Write(std::ostream &file) const;
  std::istream &Read(std::istream &file);

  const char *ClassName() const;

  std::size_t HashValue() const;

 private:
  NFmiPoint itsPlace;
  NFmiPoint itsSize;

};  // class NFmiRect

//! Undocumented, should be removed
typedef NFmiRect *PNFmiRect;

// ----------------------------------------------------------------------
/*!
 * Void constructor
 */
// ----------------------------------------------------------------------

inline NFmiRect::NFmiRect() : itsPlace(NFmiPoint()), itsSize(NFmiPoint()) {}
// ----------------------------------------------------------------------
/*!
 * \param theTopLeftCorner Undocumented
 * \param theBottomRightCorner Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiRect::Set(const NFmiPoint &theTopLeftCorner, const NFmiPoint &theBottomRightCorner)
{
  itsPlace = theTopLeftCorner;
  itsSize = theBottomRightCorner - theTopLeftCorner;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline double NFmiRect::Top() const
{
  return itsPlace.Y();
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline double NFmiRect::Bottom() const
{
  return itsPlace.Y() + itsSize.Y();
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline double NFmiRect::Left() const
{
  return itsPlace.X();
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline double NFmiRect::Right() const
{
  return itsPlace.X() + itsSize.X();
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const NFmiPoint NFmiRect::TopLeft() const
{
  return itsPlace;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const NFmiPoint NFmiRect::BottomRight() const
{
  return itsPlace + itsSize;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const NFmiPoint NFmiRect::TopRight() const
{
  return NFmiPoint(itsPlace.X() + itsSize.X(), itsPlace.Y());
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const NFmiPoint NFmiRect::BottomLeft() const
{
  return NFmiPoint(itsPlace.X(), itsPlace.Y() + itsSize.Y());
}

// ----------------------------------------------------------------------
/*!
 * \param cornerIdent Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const NFmiPoint NFmiRect::Corner(FmiDirection cornerIdent) const
{
  switch (cornerIdent)
  {
    case kTopLeft:
    case kUpLeft:
      return TopLeft();
    case kRightDown:
    case kBottomRight:
      return BottomRight();
    case kLeftDown:
    case kBottomLeft:
      return BottomLeft();
    case kUpRight:
    case kTopRight:
      return TopRight();
    default:
      return NFmiPoint(kFloatMissing, kFloatMissing);
  }
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const NFmiPoint &NFmiRect::Size() const
{
  return itsSize;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const NFmiPoint &NFmiRect::Place() const
{
  return itsPlace;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline double NFmiRect::Height() const
{
  return itsSize.Y();
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline double NFmiRect::Width() const
{
  return itsSize.X();
}
// ----------------------------------------------------------------------
/*!
 * \param theValue Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiRect::Top(double theValue)
{
  itsSize.Y(Bottom() - theValue);
  itsPlace.Y(theValue);
}

// ----------------------------------------------------------------------
/*!
 * \param theValue Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiRect::Bottom(double theValue)
{
  itsSize.Y(theValue - itsPlace.Y());
}
// ----------------------------------------------------------------------
/*!
 * \param theValue Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiRect::Left(double theValue)
{
  itsSize.X(Right() - theValue);
  itsPlace.X(theValue);
}

// ----------------------------------------------------------------------
/*!
 * \param theValue Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiRect::Right(double theValue)
{
  itsSize.X(theValue - itsPlace.X());
}
// ----------------------------------------------------------------------
/*!
 * \param newPlace Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiRect::Place(const NFmiPoint &newPlace)
{
  itsPlace = newPlace;
}
// ----------------------------------------------------------------------
/*!
 * \param newSize Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiRect::Size(const NFmiPoint &newSize)
{
  itsSize = newSize;
}
// ----------------------------------------------------------------------
/*!
 * \param theRelativePoint Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const NFmiPoint NFmiRect::ToAbs(const NFmiPoint &theRelativePoint) const
{
  return NFmiPoint(Left() + theRelativePoint.X() * Width(),
                   Top() + theRelativePoint.Y() * Height());
}

// ----------------------------------------------------------------------
/*!
 * \param theRelativeRect Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const NFmiRect NFmiRect::ToAbs(const NFmiRect &theRelativeRect) const
{
  return NFmiRect(Left() + theRelativeRect.Left() * Width(),
                  Top() + theRelativeRect.Top() * Height(),
                  Left() + theRelativeRect.Right() * Width(),
                  Top() + theRelativeRect.Bottom() * Height());
}

// ----------------------------------------------------------------------
/*!
 * \param thePoint Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiRect::IsInside(const NFmiPoint &thePoint) const
{
  const double errorMarginal = 0.0000001;  // 7.4.1998/Marko
  return ((Left() - thePoint.X() <= errorMarginal) && (thePoint.X() - Right() <= errorMarginal) &&
          (Top() - thePoint.Y() <= errorMarginal) && (thePoint.Y() - Bottom() <= errorMarginal));
}

// ----------------------------------------------------------------------
/*!
 * \param theRect Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiRect::IsInside(const NFmiRect &theRect) const
{
  return (IsInside(theRect.TopLeft()) && IsInside(theRect.BottomRight()));
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiRect::IsEmpty() const
{
  return (itsSize.X() == 0 && itsSize.Y() == 0);
}
// ----------------------------------------------------------------------
/*!
 * \param theRect Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiRect::operator==(const NFmiRect &theRect) const
{
  return (itsPlace == theRect.itsPlace && itsSize == theRect.itsSize);
}

// ----------------------------------------------------------------------
/*!
 * \param theRect Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiRect::operator!=(const NFmiRect &theRect) const
{
  return !(itsPlace == theRect.itsPlace && itsSize == theRect.itsSize);
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const NFmiPoint NFmiRect::Center() const
{
  return NFmiPoint(itsPlace.X() + (Width() / 2.), itsPlace.Y() + (Height() / 2.));
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const char *NFmiRect::ClassName() const
{
  return "NFmiRect";
}
// ----------------------------------------------------------------------
/*!
 * Output operator for class NFmiRect
 *
 * \param os The output stream to write to
 * \param item The object to write
 * \return The output stream written to
 */
// ----------------------------------------------------------------------

inline std::ostream &operator<<(std::ostream &os, const NFmiRect &item)
{
  return item.Write(os);
}
// ----------------------------------------------------------------------
/*!
 * Input operator for class NFmiRect
 *
 * \param is The input stream to read from
 * \param item The object into which to read the new contents
 * \return The input stream read from
 */
// ----------------------------------------------------------------------

inline std::istream &operator>>(std::istream &is, NFmiRect &item)
{
  return item.Read(is);
}
