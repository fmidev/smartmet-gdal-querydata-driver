// ======================================================================
/*!
 * \file NFmiDataMatrix.h
 * \brief Interface of class NFmiDataMatrix
 */
// ======================================================================
/*!
 * \class NFmiDataMatrix
 *
 * NFmiDataMatrix on 2D datamatriisi, joka tukee [x][y] notaatiota.
 *
 * Datalla ei oleteta olevan minkään laisia matemaattisia ominaisuuksia,
 * tarkoitus on ainoastaan säilyttää dataa 2-ulotteisesti.
 *
 * Mallilausekkeita:
 *
 * \code
 *	NFmiDataMatrix<float> matrix(nx,ny);
 *	matrix = kFloatMissing;
 *
 *	NFmiDataMatrix<float> fastermatrix(nx,ny,kFloatMissing);
 *
 *	float arvo = matrix[x][y];
 *	matrix[x][y] = arvo;
 *	float arvo2 = matrix.At(x,y,puuttuvaarvo);

 *	matrix.Resize(new_nx,new_ny,fillvalue);
 *	matrix.RemoveColumn(2);
 *
 *	cout << matrix << endl;
 * \endcode
 *
 * Note: This is the faster way to loop, i in the inside:
 *
 * \code
 *    for (int j = 0; j < matrix.NY() ; j++)
 *      for (int i = 0; i < matrix.NX(); i++)
 *	    matrix[i][j] = 2
 * \endcode
 *
 */
// ======================================================================

#pragma once

#include "NFmiGlobals.h"  // kFloatMissing
#include "NFmiStringTools.h"
#include <macgyver/Exception.h>

#include <iostream>
#include <sstream>
#include <vector>

//! A 2D data container
template <class T>  // miten annetaan containeri template parametrina??????
class NFmiDataMatrix : public std::vector<std::vector<T>>
{
 public:
  typedef typename std::vector<T>::size_type size_type;

 protected:
  size_type itsNX;  //!< Matrix width
  size_type itsNY;  //!< Matrix height

 public:
  //! Constructor

  using Base = std::vector<std::vector<T>>;

  NFmiDataMatrix(size_type nx = 0, size_type ny = 0, const T& theValue = T())
      : Base(nx, std::vector<T>(ny, theValue)), itsNX(nx), itsNY(ny)
  {
  }

  //! Copy constructor

  NFmiDataMatrix(const NFmiDataMatrix<T>& other)
      : Base(other), itsNX(other.NX()), itsNY(other.NY())
  {
  }

  ~NFmiDataMatrix() = default;

  //! Return matrix width.

  size_type NX() const { return itsNX; }
  //! Return matrix height.

  size_type NY() const { return itsNY; }

  //! Matrix value at given location, or given value outside matrix

  const T& At(int i, int j, const T& missingvalue) const
  {
    if (i < 0 || j < 0 || static_cast<size_type>(i) >= itsNX || static_cast<size_type>(j) >= itsNY)
      return missingvalue;
    else
      return this->operator[](i)[j];
  }

  const T& GetValue(int theLongIndex, const T& missingvalue) const
  {
    return At(theLongIndex % static_cast<int>(itsNX),
              theLongIndex / static_cast<int>(itsNX),
              missingvalue);
  }

  void SetValue(int theLongIndex, const T& value)
  {
    int i = theLongIndex % static_cast<int>(itsNX);
    int j = theLongIndex / static_cast<int>(itsNX);
    if (i >= 0 && j >= 0 && static_cast<size_type>(i) < itsNX && static_cast<size_type>(j) < itsNY)
    {
      this->operator[](i)[j] = value;
    }
  }

  //! Matrix value at given location, it returns reference so you can modify it, or if out-of-bounds
  //! indexies => throws

  const T& At(int i, int j) const
  {
    try
    {
      return this->at(i).at(j);
    }
    catch (std::exception& e)
    {
      DoErrorReporting(e, i, j);
    }
    throw Fmi::Exception(
        BCP,
        "Ei pitäisi mennä tähän, mutta muuten kääntäjä valittaa että funktion pitää palauttaa");
  }

  //! Matrix value at given location, it returns reference so you can modify it, or if out-of-bounds
  //! indexies => throws

  T& At(int i, int j)
  {
    try
    {
      return this->at(i).at(j);
    }
    catch (std::exception& e)
    {
      DoErrorReporting(e, i, j);
    }
    throw Fmi::Exception(
        BCP,
        "Ei pitäisi mennä tähän, mutta muuten kääntäjä valittaa että funktion pitää palauttaa");
  }

  void DoErrorReporting(std::exception& e, int i, int j) const
  {
    std::string indexStr("NFmiDataMatrix size: ");
    indexStr += NFmiStringTools::Convert(itsNX);
    indexStr += " x ";
    indexStr += NFmiStringTools::Convert(itsNY);
    indexStr += " and indexies [x][y]: ";
    indexStr += NFmiStringTools::Convert(i);
    indexStr += " and ";
    indexStr += NFmiStringTools::Convert(j);
    indexStr += "\n";
    throw Fmi::Exception(BCP, e.what() + std::string("\n") + indexStr);
  }

  //! Resize matrix to desired size, with given value for new elements.

  void Resize(size_type theNX, size_type theNY, const T& theValue = T())
  {
    if (itsNY == theNY && itsNX == theNX)
      return;

    itsNY = theNY;
    itsNX = theNX;
    this->resize(itsNX);
    for (size_type i = 0; i < itsNX; i++)
      this->operator[](i).resize(itsNY, theValue);
  }

  //! Remove the given row, e.g, matrix[*][j]

  void RemoveRow(size_type theY)
  {
    for (size_type i = 0; i < itsNX; i++)
      this->operator[](i).erase((this->operator[](i)).begin() + theY);
    itsNY--;
  }

  //! Remove the given column, e.g, matrix[i][*]

  void RemoveColumn(size_type theX)
  {
    typename NFmiDataMatrix::iterator it = this->begin();
    it += theX;
    this->erase(it);
    itsNX--;
  }

  //! Assignment operator : matrix = matrix

  NFmiDataMatrix<T>& operator=(const NFmiDataMatrix<T>& theMatrix)
  {
    Resize(theMatrix.NX(), theMatrix.NY());
    for (size_type j = 0; j < itsNY; j++)
      for (size_type i = 0; i < itsNX; i++)
        this->operator[](i)[j] = theMatrix[i][j];
    return *this;
  }

  //! Assignment operator: matrix = value

  NFmiDataMatrix<T>& operator=(const T& theValue)
  {
    for (size_type j = 0; j < itsNY; j++)
      for (size_type i = 0; i < itsNX; i++)
        this->operator[](i)[j] = theValue;
    return *this;
  }

  //! Move assignment operator: matrix = matrix

  NFmiDataMatrix<T>& operator=(NFmiDataMatrix<T>&& other)
  {
    if (&other != this)
    {
      itsNX = other.itsNX;
      itsNY = other.itsNY;
      Base::operator=(std::move(other));
    }
    return *this;
  }

  //! Data replacement operator: matrix.Replace(source,target)

  void Replace(const T& theSourceValue, const T& theTargetValue)
  {
    for (size_type j = 0; j < itsNY; j++)
      for (size_type i = 0; i < itsNX; i++)
        if (this->operator[](i)[j] == theSourceValue)
          this->operator[](i)[j] = theTargetValue;
  }

  //! Addition operator matrix += matrix

  NFmiDataMatrix<float>& operator+=(const NFmiDataMatrix<float>& theMatrix)
  {
    for (size_type j = 0; j < itsNY; j++)
      for (size_type i = 0; i < itsNX; i++)
        this->operator[](i)[j] = FloatAdd(this->operator[](i)[j], theMatrix[i][j]);
    return *this;
  }

  //! Addition operator matrix += value
  NFmiDataMatrix<float>& operator+=(float theValue)
  {
    for (size_type j = 0; j < itsNY; j++)
      for (size_type i = 0; i < itsNX; i++)
        this->operator[](i)[j] = FloatAdd(this->operator[](i)[j], theValue);
    return *this;
  }

  NFmiDataMatrix<float>& operator-=(const NFmiDataMatrix<float>& theMatrix)
  {
    for (size_type j = 0; j < itsNY; j++)
      for (size_type i = 0; i < itsNX; i++)
        this->operator[](i)[j] = FloatAdd(this->operator[](i)[j], -theMatrix[i][j]);
    return *this;
  }

  NFmiDataMatrix<float>& operator-=(float theValue)
  {
    for (size_type j = 0; j < itsNY; j++)
      for (size_type i = 0; i < itsNX; i++)
        this->operator[](i)[j] = FloatAdd(this->operator[](i)[j], -theValue);
    return *this;
  }

  NFmiDataMatrix<float>& operator*=(const NFmiDataMatrix<float>& theMatrix)
  {
    for (size_type j = 0; j < itsNY; j++)
      for (size_type i = 0; i < itsNX; i++)
        this->operator[](i)[j] = FloatMul(this->operator[](i)[j], theMatrix[i][j]);
    return *this;
  }

  NFmiDataMatrix<float>& operator*=(float theValue)
  {
    for (size_type j = 0; j < itsNY; j++)
      for (size_type i = 0; i < itsNX; i++)
        this->operator[](i)[j] = FloatMul(this->operator[](i)[j], theValue);
    return *this;
  }

  NFmiDataMatrix<float>& operator/=(const NFmiDataMatrix<float>& theMatrix)
  {
    for (size_type j = 0; j < itsNY; j++)
      for (size_type i = 0; i < itsNX; i++)
        this->operator[](i)[j] = FloatDiv(this->operator[](i)[j], theMatrix[i][j]);
    return *this;
  }

  NFmiDataMatrix<float>& operator/=(float theValue)
  {
    for (size_type j = 0; j < itsNY; j++)
      for (size_type i = 0; i < itsNX; i++)
        this->operator[](i)[j] = FloatDiv(this->operator[](i)[j], theValue);
    return *this;
  }

  void LinearCombination(const NFmiDataMatrix<float>& theMatrix, float weight1, float weight2)
  {
    for (size_type j = 0; j < itsNY; j++)
      for (size_type i = 0; i < itsNX; i++)
        this->operator[](i)[j] =
            FloatAdd(FloatMul(this->operator[](i)[j], weight1), FloatMul(theMatrix[i][j], weight2));
  }

  void Min(const NFmiDataMatrix<float>& theMatrix)
  {
    for (size_type j = 0; j < itsNY; j++)
      for (size_type i = 0; i < itsNX; i++)
        this->operator[](i)[j] = FloatMin(this->operator[](i)[j], theMatrix[i][j]);
  }

  void Min(float theValue)
  {
    for (size_type j = 0; j < itsNY; j++)
      for (size_type i = 0; i < itsNX; i++)
        this->operator[](i)[j] = FloatMin(this->operator[](i)[j], theValue);
  }

  void Max(const NFmiDataMatrix<float>& theMatrix)
  {
    for (size_type j = 0; j < itsNY; j++)
      for (size_type i = 0; i < itsNX; i++)
        this->operator[](i)[j] = FloatMax(this->operator[](i)[j], theMatrix[i][j]);
  }

  void Max(float theValue)
  {
    for (size_type j = 0; j < itsNY; j++)
      for (size_type i = 0; i < itsNX; i++)
        this->operator[](i)[j] = FloatMax(this->operator[](i)[j], theValue);
  }

  void SetMinMax(float theMinValue, float theMaxValue)
  {
    for (size_type j = 0; j < itsNY; j++)
      for (size_type i = 0; i < itsNX; i++)
        this->operator[](i)[j] = FloatMinMax(this->operator[](i)[j], theMinValue, theMaxValue);
  }

 private:
  float FloatAdd(float a, float b) const
  {
    if (a == kFloatMissing || b == kFloatMissing ||
        b == -kFloatMissing)  // HUOM! pitää ottaa huomioon myös negatiivinen puuttuva, koska esim.
                              // NFmiDataMatrix operator-= antaa b-arvot tänne negatiivisina
      return kFloatMissing;
    else
      return a + b;
  }

  float FloatMul(float a, float b) const
  {
    if (a == kFloatMissing || b == kFloatMissing)
      return kFloatMissing;
    else
      return a * b;
  }

  float FloatDiv(float a, float b) const
  {
    if (a == kFloatMissing || b == kFloatMissing || b == 0.0)
      return kFloatMissing;
    else
      return a / b;
  }

  float FloatMin(float a, float b) const
  {
    if (a == kFloatMissing || b == kFloatMissing)
      return kFloatMissing;
    else
      return (a < b ? a : b);
  }

  float FloatMax(float a, float b) const
  {
    if (a == kFloatMissing || b == kFloatMissing)
      return kFloatMissing;
    else
      return (a > b ? a : b);
  }

  float FloatMinMax(float a, float minLimit, float maxLimit) const
  {
    if (a == kFloatMissing || minLimit == kFloatMissing || maxLimit == kFloatMissing)
      return kFloatMissing;
    else
    {
      a = (a > maxLimit ? maxLimit : a);
      return (a < minLimit ? minLimit : a);
    }
  }
};

template <class T>
inline std::ostream& operator<<(std::ostream& s, const NFmiDataMatrix<T>& m)
{
  typedef typename NFmiDataMatrix<T>::size_type sz_type;
  sz_type rows = m.NY();
  sz_type columns = m.NX();

  s << static_cast<unsigned int>(columns) << " " << static_cast<unsigned int>(rows) << std::endl;

  for (sz_type j = 0; j < rows; j++)
  {
    for (sz_type i = 0; i < columns; i++)
      s << m[i][j] << " ";
    s << std::endl;
  }
  return s;
}

template <class T>
inline std::istream& operator>>(std::istream& s, NFmiDataMatrix<T>& m)
{
  typedef typename NFmiDataMatrix<T>::size_type sz_type;
  sz_type rows = 0;
  sz_type columns = 0;

  s >> columns >> rows;
  m.Resize(columns, rows);
  for (sz_type j = 0; j < rows; j++)
  {
    for (sz_type i = 0; i < columns; i++)
      s >> m[i][j];
  }
  return s;
}

// ======================================================================
