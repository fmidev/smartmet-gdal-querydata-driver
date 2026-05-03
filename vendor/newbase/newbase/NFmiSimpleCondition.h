#pragma once

#include "NFmiAreaMask.h"
#include <memory>

class NFmiAreaMask;
class NFmiCalculationParams;

// Part luokka pitää sisällään joko yhtä maskia (esim. T_ec), jolta voidaan pyytää arvoa.
// Tai siinä on kaksi maksia ja niiden välissä oleva lasku operaatio (esim. T_ec - Td_ec).
class NFmiSimpleConditionPart
{
  std::shared_ptr<NFmiAreaMask> itsMask1;
  // Jos stationaarista, ei saa tehdä aikainterpolaatiota
  bool isMask1StationaryData = false;
  NFmiAreaMask::CalculationOperator itsCalculationOperator = NFmiAreaMask::NotOperation;
  std::shared_ptr<NFmiAreaMask> itsMask2;
  // Jos stationaarista, ei saa tehdä aikainterpolaatiota
  bool isMask2StationaryData = false;
  // Tätä käytetään hyväksi -> vertailuoperaattorin kanssa eli ns. continuous-equal laskuissa
  double itsPreviousValue = kFloatMissing;

 public:
  ~NFmiSimpleConditionPart();
  NFmiSimpleConditionPart(std::shared_ptr<NFmiAreaMask> &mask1,
                          NFmiAreaMask::CalculationOperator calculationOperator,
                          std::shared_ptr<NFmiAreaMask> &mask2);
  NFmiSimpleConditionPart(const NFmiSimpleConditionPart &theOther);
  void Initialize();
  NFmiSimpleConditionPart *Clone() const;
  NFmiSimpleConditionPart &operator=(const NFmiSimpleConditionPart &) = delete;

  std::shared_ptr<NFmiAreaMask> Mask1() const { return itsMask1; }
  NFmiAreaMask::CalculationOperator CalculationOperator() const { return itsCalculationOperator; }
  std::shared_ptr<NFmiAreaMask> Mask2() const { return itsMask2; }

  // Eri tapauksia varten omat tarkastelu funktiot
  double Value(const NFmiCalculationParams &theCalculationParams, bool fUseTimeInterpolationAlways);
  double PressureValue(double thePressure, const NFmiCalculationParams &theCalculationParams);
  double HeightValue(double theHeight, const NFmiCalculationParams &theCalculationParams);

  double PreviousValue(double newPreviousValue);
  void ResetPreviousValue();
};

// Class is used by smarttool language. Some smarttool functions may have this
// simple-condition type that is calculated before smarttool function does
// integration calculations on certain calculation point.
// This condition is given in form "var1 operand var2", where var1/2 are
// either parameter (T_ec), variable (var x = ?) or constant (5).
class NFmiSingleCondition
{
 public:
  ~NFmiSingleCondition();
  NFmiSingleCondition(const std::shared_ptr<NFmiSimpleConditionPart> &thePart1,
                      FmiMaskOperation theConditionOperand1,
                      const std::shared_ptr<NFmiSimpleConditionPart> &thePart2,
                      FmiMaskOperation theConditionOperand2 = kFmiNoMaskOperation,
                      const std::shared_ptr<NFmiSimpleConditionPart> &thePart3 = nullptr);
  NFmiSingleCondition(const NFmiSingleCondition &theOther);
  // Tätä kutsutaan konstruktorin jälkeen, tässä alustetaan ainakin tieto siitä onko maski ns.
  // stationaaristä dataa
  void Initialize();
  NFmiSingleCondition *Clone() const;
  NFmiSingleCondition &operator=(const NFmiSingleCondition &) = delete;

  // Eri tapauksia varten omat tarkastelu funktiot
  bool CheckCondition(const NFmiCalculationParams &theCalculationParams,
                      bool fUseTimeInterpolationAlways);
  bool CheckPressureCondition(double thePressure,
                              const NFmiCalculationParams &theCalculationParams);
  bool CheckHeightCondition(double theHeight, const NFmiCalculationParams &theCalculationParams);

  void ResetPreviousValue();

 protected:
  // part1 and part2 are always present, because they form basic simple condition:
  // part1 condition part2 e.g. T_ec - x > RH_ec * 1.3 where T_ec would be NFmiInfoAreaMask object,
  // x would be variable (NFmiInfoAreaMask object) and 1.3 would be NFmiCalculationConstantValue
  // object.
  std::shared_ptr<NFmiSimpleConditionPart> part1;
  // Condition between part1 and part2, always present
  FmiMaskOperation conditionOperand1;
  std::shared_ptr<NFmiSimpleConditionPart> part2;
  // Condition between part2 and part3, present only if there is part3 and we are dealing with range
  // case. Range conditions could be inside (like -5 < T_ec < 0 meaning between values -5 and 0) or
  // outside (-5 > T_ec > 0 meaning under -5 and over 0)
  FmiMaskOperation conditionOperand2;
  // part3 is only present if there is range case.
  std::shared_ptr<NFmiSimpleConditionPart> part3;
};

// Simple-condition koostuu yhdestä tai kahdesta single-conditionista. ne yhdistetään
// tarvittaessa loogisella operaattorilla (and/or/xor).
// Esim. Ec lämpötila on positiivinen ja ollaan maalla: "T_ec > 0 and topo > 0"
class NFmiSimpleCondition
{
 public:
  ~NFmiSimpleCondition();
  NFmiSimpleCondition(const std::shared_ptr<NFmiSingleCondition> &theCondition1,
                      NFmiAreaMask::BinaryOperator theConditionOperator,
                      const std::shared_ptr<NFmiSingleCondition> &theCondition2);
  NFmiSimpleCondition(const NFmiSimpleCondition &theOther);
  // Tätä kutsutaan konstruktorin jälkeen, tässä alustetaan ainakin tieto siitä onko maski ns.
  // stationaaristä dataa
  void Initialize();
  NFmiSimpleCondition *Clone() const;
  NFmiSimpleCondition &operator=(const NFmiSimpleCondition &) = delete;

  // Eri tapauksia varten omat tarkastelu funktiot
  bool CheckCondition(const NFmiCalculationParams &theCalculationParams,
                      bool fUseTimeInterpolationAlways);
  bool CheckPressureCondition(double thePressure,
                              const NFmiCalculationParams &theCalculationParams);
  bool CheckHeightCondition(double theHeight, const NFmiCalculationParams &theCalculationParams);

  void ResetPreviousValue();

 protected:
  // condition1 on aina mukana (esim. T_ec > 0)
  std::shared_ptr<NFmiSingleCondition> condition1;
  // conditionOperator ja condition2 mukana tietyissä tapauksissa
  // Esim. "T_ec >0 and topo > 0", missä "and" on conditionOperator ja "topo > 0" on condition2
  NFmiAreaMask::BinaryOperator conditionOperator;
  std::shared_ptr<NFmiSingleCondition> condition2;
};
