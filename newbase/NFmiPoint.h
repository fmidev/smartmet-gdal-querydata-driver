// ======================================================================
/*!
 * \file NFmiPoint.h
 * \brief Interface of class NFmiPoint
 */
// ======================================================================

#pragma once

#include "NFmiGlobals.h"
#include <cmath>
#include <iosfwd>
#include <tuple>

class NFmiPoint
{
 public:
  static const NFmiPoint gMissingLatlon;  // Tätä missing latlon-pistettä voi käyttää, jos haluaa
                                          // ilmaista että jokin latlon-piste ei mahdollisesti ole
                                          // initialisoitu

  NFmiPoint();
  NFmiPoint(double theX, double theY);
  NFmiPoint(const std::pair<double, double> &theXY) : itsX(theXY.first), itsY(theXY.second) {}

  // Methods left for optimized compiler generation:
  // ~NFmiPoint();
  // NFmiPoint(const NFmiPoint & thePoint);
  // NFmiPoint & operator=(const NFmiPoint & thePoint);

  //@{ \name Asetus-funktiot
  void Set(double newX, double newY);
  void X(double newValue);
  void Y(double newValue);
  //@}

  //@{ \name Palautus-funktiot
  double X() const;
  double Y() const;
  //@}

  //@{ \name Yleiset vertailu- ja lasku-operaatiot
  bool operator==(const NFmiPoint &thePoint) const;
  bool operator!=(const NFmiPoint &thePoint) const;
  bool operator<(const NFmiPoint &thePoint) const;
  NFmiPoint &operator+=(const NFmiPoint &thePoint);
  NFmiPoint &operator-=(const NFmiPoint &thePoint);
  NFmiPoint &operator*=(const NFmiPoint &thePoint);
  NFmiPoint &operator/=(const NFmiPoint &thePoint);
  //@}

  double Distance(const NFmiPoint &thePoint) const;

  FmiDirection DirectionOfDifference(const NFmiPoint &thePoint) const;

  //@{ \name Kirjoitus- ja luku-operaatiot
  std::ostream &Write(std::ostream &file) const;
  std::istream &Read(std::istream &file);
  //@}

  const char *ClassName() const;

  std::size_t HashValue() const;

 private:
  double itsX;  //!< Pisteen X-sijainti.
  double itsY;  //!< Pisteen Y-sijainti.

};  // class NFmiPoint

//! Undocumented, should be removed
typedef NFmiPoint *PNFmiPoint;

// ----------------------------------------------------------------------
/*!
 * Void constructor initializes the coordinates to (0,0)
 */
// ----------------------------------------------------------------------

inline NFmiPoint::NFmiPoint() : itsX(0.0), itsY(0.0) {}
// ----------------------------------------------------------------------
/*!
 * Constructor with given coordinates
 *
 * \param theX The X-coordinate
 * \param theY The Y-coordinate
 */
// ----------------------------------------------------------------------

inline NFmiPoint::NFmiPoint(double theX, double theY) : itsX(theX), itsY(theY) {}
// ----------------------------------------------------------------------
/*!
 * Equality comparison
 *
 * \param thePoint The object to which \c this is compared
 * \return True if the objects are equal
 */
// ----------------------------------------------------------------------

inline bool NFmiPoint::operator==(const NFmiPoint &thePoint) const
{
  return (itsX == thePoint.itsX && itsY == thePoint.itsY);
}

// ----------------------------------------------------------------------
/*!
 * Inequality comparison
 *
 * \param thePoint The object to which \c this is compared
 * \return True if the objects are not equal
 */
// ----------------------------------------------------------------------

inline bool NFmiPoint::operator!=(const NFmiPoint &thePoint) const
{
  return !(itsX == thePoint.itsX && itsY == thePoint.itsY);
}

// ----------------------------------------------------------------------
/*!
 * \brief Lexicographic less-than comparison
 *
 * This is implemented solely for the benefit of providing a strict
 * ordering of points so that the objects may be placed in standard
 * associative containers (set,map).
 *
 * We choose the X-component to be lexicographically before the
 * Y-component.
 *
 * \param thePoint The object to which \c this is compared
 * \return True, if \c this is lexicographically less than thePoint
 */
// ----------------------------------------------------------------------

inline bool NFmiPoint::operator<(const NFmiPoint &thePoint) const
{
  return (itsX != thePoint.itsX ? itsX < thePoint.itsX : itsY < thePoint.itsY);
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline double NFmiPoint::X() const
{
  return itsX;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline double NFmiPoint::Y() const
{
  return itsY;
}
// ----------------------------------------------------------------------
/*!
 * \param newValue Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiPoint::X(double newValue)
{
  itsX = newValue;
}
// ----------------------------------------------------------------------
/*!
 * \param newValue Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiPoint::Y(double newValue)
{
  itsY = newValue;
}
// ----------------------------------------------------------------------
/*!
 * Returns the Euclidian distance between \c this and the given point.
 *
 * The formula of the distance is
 *
 * \f$D = \sqrt{((x_1-x_2)^2 + (y_1-y_2)^2)}\f$
 *
 * \param thePoint The other point
 * \return The distance between the points
 */
// ----------------------------------------------------------------------

inline double NFmiPoint::Distance(const NFmiPoint &thePoint) const
{
  using namespace std;
  return double(sqrt((itsX - thePoint.X()) * (itsX - thePoint.X()) +
                     (itsY - thePoint.Y()) * (itsY - thePoint.Y())));
}

// ----------------------------------------------------------------------
/*!
 * Addition of two NFmiPoint objects
 *
 * \param leftPoint Point number 1
 * \param rightPoint Point number 2
 * \return The point with the added coordinates
 */
// ----------------------------------------------------------------------

inline NFmiPoint operator+(const NFmiPoint &leftPoint, const NFmiPoint &rightPoint)
{
  return NFmiPoint(leftPoint.X() + rightPoint.X(), leftPoint.Y() + rightPoint.Y());
}

// ----------------------------------------------------------------------
/*!
 * Substraction of two NFmiPoint objects
 *
 * \param leftPoint Point number 1
 * \param rightPoint Point number 2
 * \return The point with the substracted coordinates
 */
// ----------------------------------------------------------------------

inline NFmiPoint operator-(const NFmiPoint &leftPoint, const NFmiPoint &rightPoint)
{
  return NFmiPoint(leftPoint.X() - rightPoint.X(), leftPoint.Y() - rightPoint.Y());
}

// ----------------------------------------------------------------------
/*!
 * Scale a NFmiPoint object
 */
// ----------------------------------------------------------------------

inline NFmiPoint operator*(const NFmiPoint &leftPoint, double rightScale)
{
  return NFmiPoint(leftPoint.X() * rightScale, leftPoint.Y() * rightScale);
}

// ----------------------------------------------------------------------
/*!
 * Scale a NFmiPoint object
 */
// ----------------------------------------------------------------------

inline NFmiPoint operator*(double leftScale, const NFmiPoint &rightPoint)
{
  return NFmiPoint(leftScale * rightPoint.X(), leftScale * rightPoint.Y());
}

// ----------------------------------------------------------------------
/*!
 * Returns the ASCII name of the class
 *
 * \return "NFmiPoint"
 */
// ----------------------------------------------------------------------

inline const char *NFmiPoint::ClassName() const
{
  return "NFmiPoint";
}
// ----------------------------------------------------------------------
/*!
 * Output operator for class NFmiPoint.
 *
 * \param os The output stream to write to
 * \param item The object to write
 * \return The output stream written to
 */
// ----------------------------------------------------------------------

inline std::ostream &operator<<(std::ostream &os, const NFmiPoint &item)
{
  return (*const_cast<NFmiPoint *>(&item)).Write(os);
}

// ----------------------------------------------------------------------
/*!
 * Input operator for class NFmiPoint.
 *
 * \param is The input stream to read from
 * \param item The object into which to read the new contents
 * \return The input stream read from
 */
// ----------------------------------------------------------------------

inline std::istream &operator>>(std::istream &is, NFmiPoint &item)
{
  return item.Read(is);
}

// ======================================================================
