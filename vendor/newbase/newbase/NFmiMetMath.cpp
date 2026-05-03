// ======================================================================
/*!
 * \file
 * \brief Implementation of various meteorological functions
 */
// ======================================================================

#include "NFmiMetMath.h"
#include <macgyver/Exception.h>

using namespace std;

namespace
{
const float Kelvin0 = 273.15f;
const float simmer_limit = 14.5;
}  // namespace

// ----------------------------------------------------------------------
/*!
 * Convert degrees in Celsius to respective degrees in Fahrenheit
 *
 * \param value Degrees Celsius
 * \return Respective degrees Fahrenheit
 */
// ----------------------------------------------------------------------

float FmiCelsius2Fahrenheit(float value)
{
  try
  {
    return value == kFloatMissing ? kFloatMissing : value * 1.8f + 32.f;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Calculate dewpoint temperature
 *
 * \param t The temperature in degrees Celsius
 * \param rh The relative humidity in range 0-100
 * \return The approximated dewpoint temperature
 */
// ----------------------------------------------------------------------

float FmiDewPoint(float t, float rh)
{
  try
  {
    return FmiDewPointFMI(t, rh);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}
// ----------------------------------------------------------------------
/*!
 * \brief Calculate dewpoint temperature
 *
 * The formula is from
 * <a href="http://meted.ucar.edu/awips/validate/dewpnt.htm">UCAR</a>
 * and is based on the integrated form of the Clausius-Clapeyron equation.
 *
 * The best accuracy is obtained in range -25C to +35C
 *
 * \param t The temperature in degrees Celsius
 * \param rh The relative humidity in range 0-100
 * \return The approximated dewpoint temperature
 */
// ----------------------------------------------------------------------

float FmiDewPointUCAR(float t, float rh)
{
  try
  {
    if (t == kFloatMissing || rh == kFloatMissing)
      return kFloatMissing;

    static const float C15 = 26.66082f;
    static const float C1 = 0.0091379024f;
    static const float C2 = 6106.396f;
    static const float C3 = 223.1986f;
    static const float C4 = 0.0182758048f;

    const float T = t + Kelvin0;   // from Celsius to Kelvins
    const float RH = rh / 100.0f;  // ratio 0-1

    const float es = exp(C15 - C1 * T - C2 / T);  // saturation vapor pressure
    const float e = RH * es;                      // vapor pressure

    const float b = C15 - log(e);
    const float tdew = (b - sqrt(b * b - C3)) / C4;

    return tdew - Kelvin0;  // back to Celsius
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Calculate dewpoint temperature
 *
 * The formula is from Marko Pietarinen, original source unknown.
 *
 * \param t The temperature in degrees Celsius
 * \param rh The relative humidity in range 0-100
 * \return The approximated dewpoint temperature
 */
// ----------------------------------------------------------------------

float FmiDewPointFMI(float t, float rh)
{
  try
  {
    if (t == kFloatMissing || rh == kFloatMissing)
      return kFloatMissing;

    static const float b = 17.27f;
    static const float c = 237.3f;

    const float x = (log(rh / 100.0f) + b * (t / (t + c))) / b;
    const float tdew = c * x / (1 - x);

    return tdew;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Calculate water to snow conversion factor
 *
 * The formula is based on the table given on page 12 of the
 * M.Sc thesis by Ivan Dube titled "From mm to cm... Study of
 * snow/liquid water ratios in Quebec".
 *
 * \param t The air temperature
 * \param ff The wind speed in m/s
 * \return The snow/water ratio
 */
// ----------------------------------------------------------------------

float FmiSnowWaterRatio(float t, float ff)
{
  try
  {
    if (t == kFloatMissing || ff == kFloatMissing)
      return 10.0;

    const float knot = 0.51444444444444444444f;

    if (ff < 10 * knot)
    {
      if (t > 0)
        return 10;
      else if (t > -5)
        return 11;
      else if (t > -10)
        return 14;
      else if (t > -20)
        return 17;
      else
        return 15;
    }
    else if (ff < 20 * knot)
    {
      if (t > -5)
        return 10;
      else if (t > -10)
        return 12;
      else if (t > -20)
        return 15;
      else
        return 13;
    }
    else
    {
      if (t > -10)
        return 10;
      else if (t > -15)
        return 11;
      else if (t > -20)
        return 14;
      else
        return 12;
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Calculate the lower limit of water to snow conversion
 *
 * The conversion is trivial. See https://jira.fmi.fi/browse/SOL-974
 *
 * \param p The precipitation amount in mm
 *
 * \return The lower limit of snow precipitation amount
 */
// ----------------------------------------------------------------------

float FmiSnowLowerLimit(float prec)
{
  try
  {
    float snowLowerLimitFactor = 7.0;
    return prec * snowLowerLimitFactor;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Calculate the upper limit of water to snow conversion
 *
 * The conversion is trivial. See https://jira.fmi.fi/browse/SOL-974
 *
 * \param p The precipitation amount in mm
 *
 * \return The upper limit of snow precipitation amount
 */
// ----------------------------------------------------------------------

float FmiSnowUpperLimit(float prec)
{
  try
  {
    float snowUpperLimitFactor = 15.0;
    return prec * snowUpperLimitFactor;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Calculate pressure level from given flight-level
 *
 * FL-korkeudet laskukaavat:
 * http://amdar.wmo.int/Publications/AMDAR_Reference_Manual_2003.pdf
 * (sivu 8) mukaan lentopintaa vastaava painepinta saadaan kaavoilla (PALT = pressure altitude)
 *
 * PALT <= 36089ft:   p(hPa) = 1013.25(1 - 10^-6 * 6.8756(PALT))^5.2559
 * PALT > 36089ft:     p(hPa) = 226.32exp-((PALT-36089)/20805)
 * PALT on lentopinta jalkoina eli FL150 -> PALT = 15000 [ft]
 * tulos on paine [hPA/mbar]
 *
 * \param PALT on lentopinta jalkoina
 * \return the calculated pressure value [hPa] or missing value.
 */
// ----------------------------------------------------------------------

double CalcFlightLevelPressure(double PALT)
{
  try
  {
    if (PALT <= 36089)
      return 1013.25 * ::pow((1 - (0.000001 * 6.8756 * PALT)), 5.2559);
    else
      return 226.32 * ::exp(-((PALT - 36089) / 20805));
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Calculate filght level from pressure level
 *
 * Calculates flight level [FL] from given pressure level [hPa]
 * By definition,
 * "The U.S. Standard Atmosphere, ICAO Standard Atmosphere and WMO
 * (World Meteorological Organization)
 * standard atmospheres are the same as the ISO International Standard
 * Atmosphere for altitudes up to 32 km"
 *
 * Refs:
 *  [1]    Comparison of Boundary-Layer Profiles and Layer Detection
 *         by AMDAR andWTR/RASS at Frankfurt Airport
 * [2]     Painting JD (2003) AMDAR reference manual. Technical report
 *         WMO-no 958. World Meteorological Organization, Geneva, 84 pp
 */
// ----------------------------------------------------------------------

double CalcPressureFlightLevel(double hPa)
{
  try
  {
    if (hPa == kFloatMissing)
      return kFloatMissing;

    double ft = (1. - ::pow(hPa / 1013.25, 0.1903)) * 1.0e6 / 6.8756;  // [ft]
    return ft / 100.;                                                  // [FL]
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Calculate pressure at given height in standard atmosphere
 *
 * Laskee annetun kilometri korkeuden vastaavan painepinnan
 * standardi ilmakehässä, käyttäen Hypsometric Equationia
 * http://hurri.kean.edu/~yoh/calculations/hydrostatic/home.html
 * p2 = p1*exp(-g/RT*(z2-z1))
 */
// ----------------------------------------------------------------------

double CalcPressureAtHeight(double z2)
{
  try
  {
    // mean layer temperature T is calculated so that it is 15 celsius
    // at surface and drops by 6.5 c/km

    double g = 9.80665;  // acceleration due to gravity (= 9.80665 m s-2)
    double R = 287.04;   // R is the dry air gas constant (=287.04 J kg-1 K-1)
    // mean layer temperature T is calculated so that it is 15 celsius at surface and drops by 6.5
    // c/km
    double L = -6.5;  // standardiilmakehän lapserate eli lämpötilan muutos kilometreissa alhaalta
                      // ylöspäin mentäessä.
    double T0 = 273.15;             // muunnos siirto celsius -> kelvin
    double Tg = 15;                 // standardiilmakehän maanpinnan lämpötila celsiuksinä.
    double T = (Tg + z2 * L) / 2.;  // siis keski arvo lämpötila välillä 0-z2 km
    T += T0;                        // muunto kelvineiksi

    double p1 = 1013;
    double z1 = 0;
    double p2 = p1 * ::exp(-g / (R * T) * (z2 - z1) * 1000);
    return p2;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Inverse of calculating standard pressure at given height
 *
 * KÃ¤Ã¤nteinen funktio CalcPressureAtHeight:lle. AlkuperÃ¤inen funktio
 * on NFmiCrossSectionView.cpp -tiedostossa.
 */
// ----------------------------------------------------------------------

double CalcHeightAtPressure(double p2)
{
  try
  {
    double g = 9.80665;  // acceleration due to gravity (= 9.80665 m s-2)
    double R = 287.04;   // R is the dry air gas constant (=287.04 J kg-1 K-1)
    double p1 = 1013;    // standardi ilmakehän maanpintapaine
    double z1 = 0;
    double L = -6.5;     // standardi ilmakehän lapsrate eli lämpötilan lasku kelvineinä/km
    double T0 = 273.15;  // C -> kelvin muutos vakio
    double Tg = 15;      // standardi ilmakehän pintalämpötila on 15 astetta celsiusta

    double alfa = (-R / (g * 1000)) * ::log(p2 / p1);
    double z2 = (2 * z1 + alfa * (Tg + 2 * T0)) / (2 - alfa * L);
    return z2;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param value Mean 24h temperature
 * \param month The month of the temperature measurement
 * \return Undocumented
 */
// ----------------------------------------------------------------------

float FmiDegreeDays(float value, int month)
{
  try
  {
    // huom eri kynnysarvo alku/loppuvuodelle (sÃ¤teilyn vaikutus)
    if (value == kFloatMissing)
      return kFloatMissing;
    else if ((value > 12.) || (value > 10. && month < 7))
      return 0;
    else
      return 17.f - value;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Return the summer simmer index
 *
 */
// ----------------------------------------------------------------------

float FmiSummerSimmerIndex(float rh, float t)
{
  try
  {
    // The chart is vertical at this temperature by 0.1 degree accuracy
    if (t <= simmer_limit)
      return t;

    // Missing doesn't matter until now that temp > simmer_limit
    if (rh == kFloatMissing || t == kFloatMissing)
      return kFloatMissing;

    // SSI

    // const float rh_ref = 10.0/100.0;	// deserts

    // When in Finland and when > 14.5 degrees, 60% is approximately
    // the minimum mean monthly humidity. However, Google wisdom
    // claims most humans feel most comfortable either at 45%, or
    // alternatively somewhere between 50-60%. Hence we choose
    // the middle ground 50%

    const float rh_ref = 50.0 / 100.0;

    const float r = rh / 100.0;

    return (1.8 * t - 0.55 * (1 - r) * (1.8 * t - 26) - 0.55 * (1 - rh_ref) * 26) /
           (1.8 * (1 - 0.55 * (1 - rh_ref)));
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Return the wind chill, e.g., the equivalent no-wind temperature
 * felt by a human for the given wind speed.
 *
 * The formula is the new official one at FMI taken into use in 12.2003.
 * See: http://climate.weather.gc.ca/climate_normals/normals_documentation_e.html
 *
 * Note that Canadian formula uses km/h:
 *
 * W = 13.12 + 0.6215 × Tair - 11.37 × V10^0.16 + 0.3965 × Tair × V10^0.16
 * W = Tair + [(-1.59 + 0.1345 × Tair)/5] × V10m, when V10m < 5 km/h
 *
 * \param wind The observed wind speed in m/s
 * \param temp The observed temperature in degrees Celsius
 * \return Equivalent no-wind temperature
 */
// ----------------------------------------------------------------------

float FmiWindChill(float wind, float temp)
{
  try
  {
    if (wind == kFloatMissing || temp == kFloatMissing || wind < 0.f)
      return kFloatMissing;

    float kmh = wind * 3.6f;

    if (kmh < 5.0f)
      return temp + (-1.59f + 0.1345f * temp) / 5.0f * kmh;

    float wpow = std::pow(kmh, 0.16f);

    return 13.12f + 0.6215f * temp - 11.37f * wpow + 0.3965f * temp * wpow;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Return the wind chill, e.g., the equivalent no-wind temperature
 * felt by a human for the given wind speed.
 *
 * \param wind The observed wind speed
 * \param temp The observed temperature
 * \return Equivalent no-wind temperature
 */
// ----------------------------------------------------------------------

float FmiOldWindChill(float wind, float temp)
{
  try
  {
    // Njurmelta kaava; Vakiot naimisissa edellisen minTuulen 1.7 kanssa
    return FmiOldConvectiveHumanHeatFlux(wind, temp) == kFloatMissing
               ? kFloatMissing
               : 28.505f - .04f * FmiOldConvectiveHumanHeatFlux(wind, temp);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Return a feels-like temperature
 */
// ----------------------------------------------------------------------

float FmiFeelsLikeTemperature(float wind, float rh, float temp, float rad)
{
  try
  {
    if (temp == kFloatMissing || wind == kFloatMissing || rh == kFloatMissing)
      return kFloatMissing;

    // Calculate adjusted wind chill portion. Note that even though
    // the Canadien formula uses km/h, we use m/s and have fitted
    // the coefficients accordingly. Note that (a*w)^0.16 = c*w^16,
    // i.e. just get another coefficient c for the wind reduced to 1.5 meters.

    float a = 15.0;   // using this the two wind chills are good match at T=0
    float t0 = 37.0;  // wind chill is horizontal at this T

    float chill = a + (1 - a / t0) * temp + a / t0 * std::pow(wind + 1, 0.16) * (temp - t0);

    // Heat index

    float heat = FmiSummerSimmerIndex(rh, temp);

    // Add the two corrections together

    float feels = temp + (chill - temp) + (heat - temp);

    // Radiation correction done only when radiation is available
    // Based on the Steadman formula for Apparent temperature,
    // we just inore the water vapour pressure adjustment

    if (rad != kFloatMissing)
    {
      // Chosen so that at wind=0 and rad=800 the effect is 4 degrees
      // At rad=50 the effect is then zero degrees
      float absorption = 0.07;

      feels += 0.7 * absorption * rad / (wind + 10) - 0.25;
    }

    return feels;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param wind Undocumented
 * \param temp Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

float FmiOldConvectiveHumanHeatFlux(float wind, float temp)
{
  try
  {
    if (wind == kFloatMissing || temp == kFloatMissing || wind < 0.f)
      return kFloatMissing;

    // Njurmelta kaava
    using namespace std;

    // 1.7 m/s = walking speed
    float w = FmiMax(1.7f, wind);

    return (10.47f + 12.68f * static_cast<float>(sqrt(w)) - 1.163f * w) * (33.f - temp) -
           112.5f;  // - sÃ¤teily;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Return a apparent temperature
 *
 * Ref: http://www.bom.gov.au/info/thermal_stress/
 */
// ----------------------------------------------------------------------

float FmiApparentTemperature(float wind, float rh, float temp)
{
  try
  {
    if (temp == kFloatMissing || wind == kFloatMissing || rh == kFloatMissing)
      return kFloatMissing;

    // Water vapour pressure in hPa

    float e = rh / 100 * 6.105f * exp(17.27f * temp / (237.7f + temp));

    // Apparent temperature

    float at = temp + 0.33f * e - 0.70f * wind - 4.00f;

    return at;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ======================================================================
