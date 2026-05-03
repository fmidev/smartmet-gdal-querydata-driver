// ======================================================================
/*!
 * \file NFmiLagrange.h
 * \brief Interface of class NFmiLagrange
 */
// ======================================================================

#include "NFmiDataMatrix.h"  // for kFloatMissing
#include "NFmiDef.h"
#include "NFmiGlobals.h"  // for kFloatMissing

#include <cmath>
#include <string>

#pragma once

//! Undocumented
class NFmiLagrange
{
 public:
  ~NFmiLagrange();
  NFmiLagrange();
  NFmiLagrange(const NFmiLagrange& theLagrange);
  NFmiLagrange(const double* si, const double* ti, double* yij, int sn, int tn);
  NFmiLagrange(const double* si, const double* yij, int sn);

  void Si(const double* si, bool isJustCopyNewInputData = false);
  void Ti(const double* ti, bool isJustCopyNewInputData = false);
  void Yij(const double* yij, bool isJustCopyNewInputData = false);

  // 2D interpolation:
  bool Init(const double* si, const double* ti, double* yij, int sn, int tn);
  bool UpdateData(const double* si, const double* ti, const double* yij);
  double Interpolate(double s, double t);

  // 1D interpolation:
  bool Init(const double* si, const double* yi, int sn);
  bool UpdateData(const double* si, const double* yi);
  double Interpolate(double s);

 private:
  double* Si();
  double* Ti();
  double* Yij();

  void Denominator(const std::vector<double>& xi, std::vector<double>& denominator, int n);
  void SiDenominator();
  void TiDenominator();

  double L(const std::vector<double>& xi, double x, std::vector<double>& denominator, int i, int n);
  double Ls(double s, int i);
  double Lt(double t, int i);

  int itsSn;
  int itsTn;

  std::vector<double> itsSi;
  std::vector<double> itsTi;
  std::vector<double> itsYij;

  std::vector<double> itsSiDenominator;
  std::vector<double> itsTiDenominator;

  std::vector<double> itsLs;
  std::vector<double> itsLt;

};  // class NFmiLagrange

// ======================================================================
