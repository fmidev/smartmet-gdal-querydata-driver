// ======================================================================
/*!
 * \brief Generic 2D container
 */
// ======================================================================

#pragma once

#ifdef DEBUG
#define DEFAULT_SAFETY true
#else
#define DEFAULT_SAFETY false
#endif

#include "Exception.h"
#include <cassert>
#include <stdexcept>
#include <vector>

namespace Fmi
{
template <typename T, bool Safe = DEFAULT_SAFETY>
class Matrix
{
 public:
  using value_type = T;
  using size_type = int;

  Matrix(size_type i, size_type j, value_type value = value_type());
  Matrix(const Matrix& other);
  Matrix();

  Matrix& operator=(const Matrix& other);
  Matrix& operator=(const value_type& value);

  const value_type& operator()(size_type i, size_type j) const;
  value_type& operator()(size_type i, size_type j);

  bool empty() const { return size() == 0; }
  size_type xsize() const { return itsWidth; }
  size_type ysize() const { return itsHeight; }
  size_type size() const { return itsWidth * itsHeight; }
  // alternative APIs
  size_type width() const { return itsWidth; }
  size_type height() const { return itsHeight; }
  size_type rows() const { return itsHeight; }
  size_type columns() const { return itsWidth; }

 private:
  size_type itsWidth;
  size_type itsHeight;
  std::vector<value_type> itsData;

};  // class Matrix

// ----------------------------------------------------------------------
/*!
 * \brief Value based constructor
 */
// ----------------------------------------------------------------------

template <typename T, bool S>
inline Matrix<T, S>::Matrix(size_type i, size_type j, value_type value)
    : itsWidth(i), itsHeight(j), itsData(i * j, value)
{
}

// ----------------------------------------------------------------------
/*!
 * \brief Copy constructor
 */
// ----------------------------------------------------------------------

template <typename T, bool S>
inline Matrix<T, S>::Matrix(const Matrix& other)
    : itsWidth(other.itsWidth), itsHeight(other.itsHeight), itsData(other.itsData)
{
}

// ----------------------------------------------------------------------
/*!
 * \brief Default constructor
 */
// ----------------------------------------------------------------------

template <typename T, bool S>
inline Matrix<T, S>::Matrix() : itsWidth(0), itsHeight(0), itsData()
{
}

// ----------------------------------------------------------------------
/*!
 * \brief Assignment operator
 */
// ----------------------------------------------------------------------

template <typename T, bool S>
inline Matrix<T, S>& Matrix<T, S>::operator=(const Matrix& other)
{
  if (this != &other)
  {
    itsWidth = other.itsWidth;
    itsHeight = other.itsHeight;
    itsData = other.itsData;
  }
  return *this;
}

// ----------------------------------------------------------------------
/*!
 * \brief Assignment operator from value
 */
// ----------------------------------------------------------------------

template <typename T, bool S>
inline Matrix<T, S>& Matrix<T, S>::operator=(const value_type& value)
{
  itsData = value;
  return *this;
}

// ----------------------------------------------------------------------
/*!
 * \brief Const accessor
 */
// ----------------------------------------------------------------------

template <typename T, bool Safe>
inline const T& Matrix<T, Safe>::operator()(size_type i, size_type j) const
{
  if (Safe)
  {
    if (i < 0 || i >= itsWidth)
      throw Fmi::Exception(BCP, "X-index out of range in Matrix accessor");
    if (j < 0 || j >= itsHeight)
      throw Fmi::Exception(BCP, "Y-index out of range in Matrix accessor");
  }

  assert(i >= 0 && i < itsWidth);
  assert(j >= 0 && j < itsHeight);

  return itsData[i + itsWidth * j];
}

// ----------------------------------------------------------------------
/*!
 * \brief Non-const accessor
 */
// ----------------------------------------------------------------------

template <typename T, bool Safe>
inline T& Matrix<T, Safe>::operator()(size_type i, size_type j)
{
  if (Safe)
  {
    if (i < 0 || i >= itsWidth)
      throw Fmi::Exception(BCP, "X-index out of range in Matrix accessor");
    if (j < 0 || j >= itsHeight)
      throw Fmi::Exception(BCP, "Y-index out of range in Matrix accessor");
  }

  assert(i >= 0 && i < itsWidth);
  assert(j >= 0 && j < itsHeight);

  return itsData[i + itsWidth * j];
}

}  // namespace Fmi

// ======================================================================
