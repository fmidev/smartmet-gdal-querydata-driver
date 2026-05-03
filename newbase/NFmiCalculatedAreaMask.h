// ======================================================================
/*!
 * \file NFmiCalculatedAreaMask.h
 * \brief Interface of class NFmiCalculatedAreaMask and its derivatives
 *
 * \todo Separate individual classes into individual files
 */
// ======================================================================

#pragma once

#include "NFmiAreaMaskImpl.h"
#include "NFmiMetTime.h"

class NFmiFastQueryInfo;

//! Tarkoitus on tehdä tästä mm. auringonnousukulman ja lat/lon arvojen maskien esiluokka.

class NFmiCalculatedAreaMask : public NFmiAreaMaskImpl
{
 public:
  ~NFmiCalculatedAreaMask();
  NFmiCalculatedAreaMask();
  NFmiCalculatedAreaMask(const NFmiDataIdent& theParam,
                         const NFmiCalculationCondition& theOperation);
  NFmiCalculatedAreaMask(const NFmiCalculatedAreaMask& theMask);
  NFmiCalculatedAreaMask& operator=(const NFmiCalculatedAreaMask& theMask);
  NFmiAreaMask* Clone() const override;

  bool IsWantedParam(const NFmiDataIdent& theParam, const NFmiLevel* theLevel = 0) const override;
  const NFmiDataIdent* DataIdent() const override;
  const NFmiParam* Param() const override;

 protected:
  NFmiDataIdent* itsDataIdent;

};  // class NFmiCalculatedAreaMask

//! Undocumented
class NFmiLatLonAreaMask : public NFmiCalculatedAreaMask
{
 public:
  ~NFmiLatLonAreaMask();
  NFmiLatLonAreaMask();
  NFmiLatLonAreaMask(const NFmiDataIdent& theParam, const NFmiCalculationCondition& theOperation);
  NFmiLatLonAreaMask(const NFmiLatLonAreaMask& theMask);
  NFmiAreaMask* Clone() const override;

 protected:
  double CalcValueFromLocation(const NFmiPoint& theLatLon) const override;  // tee tämä lapsiin!!!
  const NFmiString MakeSubMaskString() const override;

 private:
};  // class NFmiLatLonAreaMask

//! Undocumented
class NFmiElevationAngleAreaMask : public NFmiLatLonAreaMask
{
 public:
  ~NFmiElevationAngleAreaMask();
  NFmiElevationAngleAreaMask();
  NFmiElevationAngleAreaMask(const NFmiElevationAngleAreaMask& theMask);
  NFmiElevationAngleAreaMask(const NFmiDataIdent& theParam,
                             const NFmiCalculationCondition& theOperation);
  bool Time(const NFmiMetTime& theTime) override;
  NFmiAreaMask* Clone() const override;
  double Value(const NFmiCalculationParams& theCalculationParams,
               bool fUseTimeInterpolationAlways) override;

 protected:
  double CalcValueFromLocation(const NFmiPoint& theLatLon) const override;  // tee tämä lapsiin!!!
  const NFmiString MakeSubMaskString() const override;
  NFmiMetTime itsTime;  // tähän aikaaan lasketaan nousukulma

};  // class NFmiElevationAngleAreaMask

//! Undocumented
class NFmiJulianDayAreaMask : public NFmiElevationAngleAreaMask
{
 public:
  ~NFmiJulianDayAreaMask();
  NFmiJulianDayAreaMask();
  NFmiJulianDayAreaMask(const NFmiJulianDayAreaMask& theMask);
  NFmiJulianDayAreaMask(const NFmiDataIdent& theParam,
                        const NFmiCalculationCondition& theOperation);
  bool Time(const NFmiMetTime& theTime) override;
  NFmiAreaMask* Clone() const override;

 protected:
  double CalcValueFromLocation(const NFmiPoint& theLatLon) const override;  // tee tämä lapsiin!!!
  const NFmiString MakeSubMaskString() const override;
  double itsJulianDay;  // tähän lasketaan Time-metodissa haluttu julian day

};  // class NFmiJulianDayAreaMask

//! Undocumented
class NFmiLocalHourAreaMask : public NFmiElevationAngleAreaMask
{
 public:
  ~NFmiLocalHourAreaMask();
  NFmiLocalHourAreaMask();
  NFmiLocalHourAreaMask(const NFmiLocalHourAreaMask& theMask);
  NFmiLocalHourAreaMask(const NFmiDataIdent& theParam,
                        const NFmiCalculationCondition& theOperation);
  NFmiAreaMask* Clone() const override;

 protected:
  double CalcValueFromLocation(const NFmiPoint& theLatLon) const override;  // tee tämä lapsiin!!!
  const NFmiString MakeSubMaskString() const override;

};  // class NFmiLocalHourAreaMask

//! Undocumented
class NFmiUtcHourAreaMask : public NFmiElevationAngleAreaMask
{
 public:
  ~NFmiUtcHourAreaMask();
  NFmiUtcHourAreaMask();
  NFmiUtcHourAreaMask(const NFmiUtcHourAreaMask& theMask);
  NFmiUtcHourAreaMask(const NFmiDataIdent& theParam, const NFmiCalculationCondition& theOperation);
  NFmiAreaMask* Clone() const override;

 protected:
  double CalcValueFromLocation(const NFmiPoint& theLatLon) const override;  // tee tämä lapsiin!!!
  const NFmiString MakeSubMaskString() const override;

};  // class NFmiUtcHourAreaMask

// class NFmiMinuteAreaMask
class NFmiMinuteAreaMask : public NFmiElevationAngleAreaMask
{
 public:
  ~NFmiMinuteAreaMask();
  NFmiMinuteAreaMask();
  NFmiMinuteAreaMask(const NFmiMinuteAreaMask& theMask);
  NFmiMinuteAreaMask(const NFmiDataIdent& theParam, const NFmiCalculationCondition& theOperation);
  NFmiAreaMask* Clone() const override;

 protected:
  double CalcValueFromLocation(const NFmiPoint& theLatLon) const override;  // tee tämä lapsiin!!!
  const NFmiString MakeSubMaskString() const override;
};

//! Undocumented
class NFmiForecastHourAreaMask : public NFmiElevationAngleAreaMask
{
 public:
  ~NFmiForecastHourAreaMask();
  NFmiForecastHourAreaMask(const std::shared_ptr<NFmiFastQueryInfo>& theInfo);
  NFmiForecastHourAreaMask(const NFmiForecastHourAreaMask& theMask);
  NFmiForecastHourAreaMask& operator=(const NFmiForecastHourAreaMask& theMask);
  NFmiForecastHourAreaMask(const std::shared_ptr<NFmiFastQueryInfo>& theInfo,
                           const NFmiDataIdent& theParam,
                           const NFmiCalculationCondition& theOperation);
  NFmiAreaMask* Clone() const override;

 protected:
  double CalcValueFromLocation(const NFmiPoint& theLatLon) const override;  // tee tämä lapsiin!!!
  const NFmiString MakeSubMaskString() const override;

 private:
  std::shared_ptr<NFmiFastQueryInfo> itsInfo;  // kysyy vain editoitavan datan ajan

};  // class NFmiForecastHourAreaMask

//! Luokka palauttaa (CalcValueFromLocation) datan timestepin tunneissa.
class NFmiTimeStepAreaMask : public NFmiElevationAngleAreaMask
{
 public:
  ~NFmiTimeStepAreaMask();
  NFmiTimeStepAreaMask(const std::shared_ptr<NFmiFastQueryInfo>& theInfo);
  NFmiTimeStepAreaMask(const NFmiTimeStepAreaMask& theMask);
  NFmiTimeStepAreaMask& operator=(const NFmiTimeStepAreaMask& theMask);
  NFmiTimeStepAreaMask(const std::shared_ptr<NFmiFastQueryInfo>& theInfo,
                       const NFmiDataIdent& theParam,
                       const NFmiCalculationCondition& theOperation);
  NFmiAreaMask* Clone() const override;
  double Value(const NFmiCalculationParams& theCalculationParams,
               bool fUseTimeInterpolationAlways) override;

 protected:
  double CalcValueFromLocation(const NFmiPoint& theLatLon) const override;  // tee tämä lapsiin!!!
  const NFmiString MakeSubMaskString() const override;

 private:
  std::shared_ptr<NFmiFastQueryInfo>
      itsInfo;  // kysyy vain editoitavan datan currentin ajan aika-askeleen tunneissa

};  // class NFmiForecastHourAreaMask

//! Luokka palauttaa (CalcValueFromLocation) datan hilan x tai y suuntaisen koon metreissä.
class NFmiGridSizeAreaMask : public NFmiElevationAngleAreaMask
{
 public:
  ~NFmiGridSizeAreaMask();
  NFmiGridSizeAreaMask(const std::shared_ptr<NFmiFastQueryInfo>& theInfo, bool calcXValue);
  NFmiGridSizeAreaMask(const std::shared_ptr<NFmiFastQueryInfo>& theInfo,
                       const NFmiDataIdent& theParam,
                       const NFmiCalculationCondition& theOperation,
                       bool calcXValue);
  NFmiGridSizeAreaMask(const NFmiGridSizeAreaMask& theMask);
  NFmiAreaMask* Clone() const override;
  double Value(const NFmiCalculationParams& theCalculationParams,
               bool fUseTimeInterpolationAlways) override;

 protected:
  double CalcValueFromLocation(const NFmiPoint& theLatLon) const override;
  const NFmiString MakeSubMaskString() const override;

 private:
  std::shared_ptr<NFmiFastQueryInfo>
      itsInfo;       // kysyy vain editoitavan datan currentin ajan aika-askeleen tunneissa
  bool fCalcXValue;  // siis jos true, laskee etäisyyden x-suunnassa, muuten y-suunnassa

  NFmiGridSizeAreaMask& operator=(const NFmiGridSizeAreaMask& theMask);
};  // class NFmiGridSizeAreaMask

// ======================================================================
