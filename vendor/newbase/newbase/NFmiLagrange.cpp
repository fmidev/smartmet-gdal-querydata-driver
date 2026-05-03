// ======================================================================
/*!
 * \file NFmiLagrange.cpp
 * \brief Implementation of class NFmiLagrange
 */
// ======================================================================
/*!
 * \class NFmiLagrange
 *
 * Computes the 1 dimensional (s) or 2 dimensional (s, t) Lagrange interpolation
 * of order (n-1) x (m-1)
 *
 * Input:
 *
 *  -# array of n independent variables s[j], j = [0..n-1] (for 1D and 2D case)
 *  -# array of m independent variables t[i], i = [0..m-1] (for 2D case only)
 *  -# array of n x m dependent variables y[i][j], j = [0..n-1], i = [0..m-1] (for 2D case only)
 *  -# array of n dependent variables y[j], j = [0..n-1], (for 1D case only)
 *
 * Output:
 *
 * Interpolated value at floating point (s, t),
 *
 *  -# s[0] < s < s[n-1], (for 1D and 2D case)
 *  -# t[0] < t < t[m-1], (for 2D case only)
 *
 * \note
 * Input grid is assumed to be rectilinear but not necessarily equidistant one.
 * Input values s[j] nor t[i] need not be given in any particular order.
 * Independent variables input should be unique, that is, not to be given twice or more.
 */
// ======================================================================

#include "NFmiLagrange.h"
#include <macgyver/Exception.h>

#include <cctype>
#include <cstdio>
#include <cstdlib>

// ----------------------------------------------------------------------
/*!
 * Void constructor
 */
// ----------------------------------------------------------------------

NFmiLagrange::NFmiLagrange()
    : itsSn(),
      itsTn(),
      itsSi(0),
      itsTi(0),
      itsYij(0),
      itsSiDenominator(0),
      itsTiDenominator(0),
      itsLs(0),
      itsLt(0)
{
}

// ----------------------------------------------------------------------
/*!
 * Constructor for 2D interpolation
 *
 * \param si Undocumented
 * \param ti Undocumented
 * \param yij Undocumented
 * \param sn Undocumented
 * \param tn Undocumented
 */
// ----------------------------------------------------------------------

NFmiLagrange::NFmiLagrange(const double *si, const double *ti, double *yij, int sn, int tn)
    : itsSn(),
      itsTn(),
      itsSi(),
      itsTi(),
      itsYij(),
      itsSiDenominator(),
      itsTiDenominator(),
      itsLs(),
      itsLt()
{
  try
  {
    Init(si, ti, yij, sn, tn);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Constructor for 1D interpolation
 *
 * \param si Undocumented
 * \param yij Undocumented
 * \param sn Undocumented
 */
// ----------------------------------------------------------------------

NFmiLagrange::NFmiLagrange(const double *si, const double *yij, int sn)
    : itsSn(),
      itsTn(),
      itsSi(),
      itsTi(),
      itsYij(),
      itsSiDenominator(),
      itsTiDenominator(),
      itsLs(),
      itsLt()
{
  try
  {
    Init(si, yij, sn);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Destructor
 */
// ----------------------------------------------------------------------

NFmiLagrange::~NFmiLagrange() = default;

// ----------------------------------------------------------------------
/*!
 * \param ti Undocumented
 * \param isJustCopyNewInputData Undocumented
 */
// ----------------------------------------------------------------------

void NFmiLagrange::Ti(const double *ti, bool isJustCopyNewInputData)
{
  try
  {
    // Set independent variable t

    if (!isJustCopyNewInputData)
    {
      // Perform a full initialization of data structures
      itsTi.resize(itsTn);
    }

    memcpy(&itsTi[0], ti, itsTn * sizeof(ti[0]));
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param si Undocumented
 * \param isJustCopyNewInputData
 */
// ----------------------------------------------------------------------

void NFmiLagrange::Si(const double *si, bool isJustCopyNewInputData)
{
  try
  {
    // Set independent variable s

    if (!isJustCopyNewInputData)
    {
      // Perform a full initialization of data structures
      itsSi.resize(itsSn);
    }

    memcpy(&itsSi[0], si, itsSn * sizeof(si[0]));
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

double *NFmiLagrange::Ti()
{
  try
  {
    // Return independent variable t

    return &itsTi[0];
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

double *NFmiLagrange::Si()
{
  try
  {
    // Return independent variable s

    return &itsSi[0];
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param yij Undocumented
 * \param isJustCopyNewInputData Undocumented
 */
// ----------------------------------------------------------------------

void NFmiLagrange::Yij(const double *yij, bool isJustCopyNewInputData)
{
  try
  {
    // Set dependent variables (m x n)

    if (!isJustCopyNewInputData)
    {
      // Perform a full initialization of data structures
      itsYij.resize(itsSn * itsTn);
    }

    memcpy(&itsYij[0], yij, itsSn * itsTn * sizeof(yij[0]));
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

double *NFmiLagrange::Yij()
{
  try
  {
    // Return dependent variables

    return &itsYij[0];
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param xi Undocumented
 * \param denominator Undocumented
 * \param n Undocumented
 */
// ----------------------------------------------------------------------

void NFmiLagrange::Denominator(const std::vector<double> &xi,
                               std::vector<double> &denominator,
                               int n)
{
  try
  {
    // Pre-compute the denominator product of the independent variables xi
    // for terms Li, i = 0.. n

    denominator.resize(n);

    for (int i = 0; i < n; i++)
    {
      // Compute i:th denominator term

      denominator[i] = 1.0;

      for (int j = 0; j < n; j++)
      {
        if (j != i)
          denominator[i] *= (xi[i] - xi[j]);
      }
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 *
 */
// ----------------------------------------------------------------------

void NFmiLagrange::SiDenominator()
{
  try
  {
    Denominator(itsSi, itsSiDenominator, itsSn);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 *
 */
// ----------------------------------------------------------------------

void NFmiLagrange::TiDenominator()
{
  try
  {
    Denominator(itsTi, itsTiDenominator, itsTn);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}
// ----------------------------------------------------------------------
/*!
 * \param xi Undocumented
 * \param x Undocumented
 * \param denominator Undocumented
 * \param i Undocumented
 * \param n Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

double NFmiLagrange::L(
    const std::vector<double> &xi, double x, std::vector<double> &denominator, int i, int n)
{
  try
  {
    // Computes the i:th term of the Lagrange polynom.
    // xi :			array of known INDEPENDENT variables
    // x  :			point where interpolation should occur at
    // denominator: pre-computed constant denominator of the product of term L.
    //				It is constant in a sense that it stays unchanged for a given array
    // of
    // xi,
    //				no matter what x is, thus permitting pre-computation
    // i  :			index of Lagrange term, i = 0..n-1
    // n  :			number of known independent variables

    double fraction = 1.0;

    for (int j = 0; j < n; j++)
    {
      if (x == xi[j])
        return 0.0;  // Trying to interpolate at control (or "observed") point!

      if (j != i)
        fraction *= (x - xi[j]);
    }

    fraction /= denominator[i];

    return fraction;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param t Undocumented
 * \param i Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

double NFmiLagrange::Lt(double t, int i)
{
  try
  {
    return L(itsTi, t, itsTiDenominator, i, itsTn);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}
// ----------------------------------------------------------------------
/*!
 * \param s Undocumented
 * \param j Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

double NFmiLagrange::Ls(double s, int j)
{
  try
  {
    return L(itsSi, s, itsSiDenominator, j, itsSn);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}
// ----------------------------------------------------------------------
/*!
 * \param s Undocumented
 * \param t Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

double NFmiLagrange::Interpolate(double s, double t)
{
  try
  {
    // Performs Lagrange 2D polynome interpolation of two independent variables s and t.
    // NOTE: input grid is assumed to be rectilinear.

    int i, j;

    // Interpolate at (s, t)

    double sum = 0.0;

    for (j = 0; j < itsSn; j++)
      itsLs[j] = Ls(s, j);

    for (i = 0; i < itsTn; i++)
    {
      itsLt[i] = Lt(t, i);

      for (j = 0; j < itsSn; j++)
      {
        sum += itsYij[itsTn * i + j] * itsLs[j] * itsLt[i];
      }
    }

    return sum;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param s Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

double NFmiLagrange::Interpolate(double s)
{
  try
  {
    // Performs Lagrange 1D polynome interpolation of one independent variable s.
    // NOTE: input grid is assumed to be rectilinear.

    // Interpolate at (s)

    double sum = 0.0;

    for (int j = 0; j < itsSn; j++)
    {
      itsLs[j] = Ls(s, j);
      sum += itsYij[j] * itsLs[j];
    }

    return sum;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param si Undocumented
 * \param ti Undocumented
 * \param yij Undocumented
 * \param sn Undocumented
 * \param tn Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiLagrange::Init(const double *si, const double *ti, double *yij, int sn, int tn)
{
  try
  {
    // 2D interpolation

    itsSn = sn;
    itsTn = tn;

    // Set input independent variables
    Ti(ti);
    Si(si);

    // Set input dependent variables
    Yij(yij);

    itsLt.resize(itsTn);

    int i = 0;
    for (i = 0; i < itsTn; i++)
      itsLt[i] = kFloatMissing;

    itsLs.resize(itsSn);

    for (i = 0; i < itsSn; i++)
      itsLs[i] = kFloatMissing;

    // Pre-compute the denominator products for terms Ls and Lt
    SiDenominator();
    TiDenominator();

    return true;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param si Undocumented
 * \param yij Undocumented
 * \param sn Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiLagrange::Init(const double *si, const double *yij, int sn)
{
  try
  {
    // 1D interpolation (only 'S' variables and arrays will be used)

    itsSn = sn;
    itsTn = 1;  // Dummy!

    // Set input independent variables
    Si(si);

    // Set input dependent variables
    Yij(yij);

    itsLs.resize(itsSn);

    for (int i = 0; i < itsSn; i++)
      itsLs[i] = kFloatMissing;

    // Pre-compute the denominator products for term Ls
    SiDenominator();

    return true;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param si Undocumented
 * \param ti Undocumented
 * \param yij Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiLagrange::UpdateData(const double *si, const double *ti, const double *yij)
{
  try
  {
    // Kind of "slight Init()" - doesn't delete nor change the size of the input data arrays
    // nor associated pre-calculated data structures but updates the input data only.

    bool isJustCopyNewInputData =
        true;  // When true, keep the existing input data sizes and pre-computed structures

    // Set input independent variables
    Ti(ti, isJustCopyNewInputData);
    Si(si, isJustCopyNewInputData);

    // Set input dependent variables
    Yij(yij, isJustCopyNewInputData);

    return true;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param si Undocumented
 * \param yij Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiLagrange::UpdateData(const double *si, const double *yij)
{
  try
  {
    // 1D interpolation.
    // Kind of "slight Init()" - doesn't delete nor change the size of the input data arrays
    // nor associated pre-calculated data structures but updates the input data only.

    bool isJustCopyNewInputData =
        true;  // When true, keep the existing input data sizes and pre-computed structures

    // Set input independent variables
    Si(si, isJustCopyNewInputData);

    // Set input dependent variables
    Yij(yij, isJustCopyNewInputData);

    return true;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ======================================================================

#ifdef TEST_MAIN

#define BINARY_OUTPUT 1

int main(int argc, char *argv[])
{
  const int n = 3;
  double z = 0.;
  double *ti = new double[n];  // Square grid --> ti = si

  ti[0] = 0;
  ti[1] = 1.;
  ti[2] = 2.;

  double *yij = new double[n * n];  // Square grid --> n = m

  yij[0] = 0.25;
  yij[1] = 0.5;
  yij[2] = 0.25;

  yij[3] = 0.5;
  yij[4] = 1.;
  yij[5] = 0.5;

  yij[6] = 0.25;
  yij[7] = 0.5;
  yij[8] = 0.25;

  double minValue = 0.25;
  double maxValue = 1.0;
  double range = maxValue - minValue;

  const int numberOfInterpolatedPoints = 100;

  double zInterpolated;

#if BINARY_OUTPUT
  ofstream inout("LagrangeInput.raw", ios::binary);
  if (!inout)
    return false;

  unsigned char gray;

  // Output the input (n x n) data as a RGB image
  for (int i = 0; i < n * n; i++)
  {
    // RGB gray scale output
    gray = (unsigned char)(255. * (yij[i] - minValue) / range);
    inout << gray << gray << gray;
  }

  inout.close();

  // Open an output file for the interpolated (numberOfInterpolatedPoints x
  // numberOfInterpolatedPoints) data - output as a RGB image
  ofstream out("LagrangeOutput.raw", ios::binary);
  if (!out)
    return false;

#else
  ofstream out("Lagrange.txt");
  if (!out)
    return false;

#endif

  NFmiLagrange laranki(ti, ti, yij, n, n);

  int j;
  double s, t;
  double tIncr = (double)(n - 1) / (double)(numberOfInterpolatedPoints -
                                            1);  // Along two intervals [t1..t2] and [t2..t3]
  double sIncr = tIncr;

  // n == m
  for (i = 0, s = 0.0; s <= (double)n, i < numberOfInterpolatedPoints; s += sIncr, i++)
  {
    for (j = 0, t = 0.0; t <= (double)n, j < numberOfInterpolatedPoints; t += tIncr, j++)
    {
      zInterpolated = laranki.Interpolate(s, t);

#if BINARY_OUTPUT
      // RGB gray scale output
      unsigned char gray = (unsigned char)(255. * (zInterpolated - minValue) / range);
      out << gray << gray << gray;
#else
      out.width(8);
      out << " (" << i << "," << j << ")= " << zInterpolated;

      // Normalized (t = [0..1], x = [0..1]) POV-RAY output:

#endif
    }

#if !BINARY_OUTPUT
    out << endl;
#endif
  }

  out.close();

  return 0;
}

#endif  // TEST_MAIN

// ======================================================================
