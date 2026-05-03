// ======================================================================
/*!
 * \file NFmiMetMath.h
 * \brief Various meteorological functions
 */
// ======================================================================

#pragma once

#include "NFmiGlobals.h"
#include <algorithm>
#include <cmath>

float FmiSnowWaterRatio(float t, float ff);
float FmiDewPointUCAR(float t, float rh);
float FmiDewPointFMI(float t, float rh);
float FmiDewPoint(float t, float rh);
float FmiCelsius2Fahrenheit(float value);
float FmiDegreeDays(float value, int month);
float FmiFeelsLikeTemperature(float wind, float rh, float temp, float rad);
float FmiWindChill(float wind, float temp);
float FmiOldWindChill(float wind, float temp);
float FmiSummerSimmerIndex(float rh, float temp);
float FmiOldConvectiveHumanHeatFlux(float wind, float temp);
float FmiApparentTemperature(float wind, float rh, float temp);
float FmiSnowLowerLimit(float prec);
float FmiSnowUpperLimit(float prec);
double CalcFlightLevelPressure(double PALT);
double CalcPressureFlightLevel(double hPa);
double CalcPressureAtHeight(double z2);
double CalcHeightAtPressure(double p2);

// ======================================================================
