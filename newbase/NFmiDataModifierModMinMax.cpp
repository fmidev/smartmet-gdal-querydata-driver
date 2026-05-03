#include "NFmiDataModifierModMinMax.h"

namespace
{
float GetModCaseMinMaxValue(const std::multiset<float>& rawValues, bool getMinBigValue)
{
  auto minBigIter = rawValues.lower_bound(180);
  if (minBigIter == rawValues.begin() || minBigIter == rawValues.end())
  {
    // minBigIter == rawValues.begin() tarkoittaa että oli vain isoja lukuja (>= 180)
    // minBigIter == rawValues.end() tarkoittaa että oli vain pieniä lukuja (< 180)
    // molemmat ovat virhetilanteita.
    return kFloatMissing;
  }
  else
  {
    if (getMinBigValue)
      return *minBigIter;
    else
      return *(--minBigIter);
  }
}

float GetMaxSmallValue(const std::multiset<float>& rawValues)
{
  return GetModCaseMinMaxValue(rawValues, false);
}

float GetMinBigValue(const std::multiset<float>& rawValues)
{
  return GetModCaseMinMaxValue(rawValues, true);
}

void TestSingleValue(float result, float expected, const std::string& testedName)
{
  if (result != expected)
  {
    std::string errorMessage = "NFmiDataModifierModMinMax tester failed: " + testedName + " value ";
    errorMessage += std::to_string(result);
    errorMessage += " was not expected ";
    errorMessage += std::to_string(expected);
    throw std::runtime_error(errorMessage);
  }
}

void TestModMinMax(const std::vector<float>& values, std::pair<float, float>& results)
{
  NFmiDataModifierModMinMax modMinMax;
  for (auto value : values)
  {
    modMinMax.Calculate(value);
  }
  auto min = modMinMax.Min();
  auto max = modMinMax.Max();
  ::TestSingleValue(min, results.first, "Min");
  ::TestSingleValue(max, results.second, "Max");
}

}  // namespace

NFmiDataModifierModMinMax::~NFmiDataModifierModMinMax() = default;

NFmiDataModifierModMinMax::NFmiDataModifierModMinMax(bool returnMinValue)
    : NFmiDataModifier(),
      itsRawValues(),
      itsMinValue(kFloatMissing),
      itsMaxValue(kFloatMissing),
      fReturnMinValue(returnMinValue)
{
}

NFmiDataModifierModMinMax::NFmiDataModifierModMinMax(const NFmiDataModifierModMinMax& theOther) =
    default;

NFmiDataModifierModMinMax* NFmiDataModifierModMinMax::Clone() const
{
  return new NFmiDataModifierModMinMax(*this);
}

void NFmiDataModifierModMinMax::Clear()
{
  itsRawValues.clear();
  itsMinValue = kFloatMissing;
  itsMaxValue = kFloatMissing;
  fCalculationsAreMade = false;
}

void NFmiDataModifierModMinMax::Calculate(float theValue)
{
  if (theValue != kFloatMissing)
  {
    itsRawValues.insert(theValue);
  }
}

float NFmiDataModifierModMinMax::CalculationResult()
{
  if (!fCalculationsAreMade) DoCalculations();
  if (fReturnMinValue)
    return itsMinValue;
  else
    return itsMaxValue;
}

float NFmiDataModifierModMinMax::Min()
{
  if (!fCalculationsAreMade) DoCalculations();
  return itsMinValue;
}

float NFmiDataModifierModMinMax::Max()
{
  if (!fCalculationsAreMade) DoCalculations();
  return itsMaxValue;
}

void NFmiDataModifierModMinMax::DoCalculations()
{
  if (!itsRawValues.empty())
  {
    if (itsRawValues.size() == 1)
    {
      itsMaxValue = itsMinValue = *itsRawValues.begin();
    }
    else
    {
      try
      {
        DoCalculations2();
      }
      catch (std::exception& e)
      {
        std::string errorMessage = "Error in NFmiDataModifierModMinMax::DoCalculations: ";
        errorMessage += e.what();
        throw std::runtime_error(errorMessage);
      }
    }
  }
  fCalculationsAreMade = true;
}

// Oletus, ollaan jo tehty tietyt tarkastelut, ja dataa on riittävästi näihin jatkolaskuihin.
void NFmiDataModifierModMinMax::DoCalculations2()
{
  auto minValue = *itsRawValues.begin();
  auto maxValue = *(--itsRawValues.end());
  auto maxMinDiff = maxValue - minValue;
  // Muutama eri tapausta tuo tähän ratkaisuun (min = min ja max = max), joten tehdään se
  // defaulttina heti alkuun
  itsMinValue = minValue;
  itsMaxValue = maxValue;
  if (maxMinDiff > 180)
  {
    auto maxSmallValue = ::GetMaxSmallValue(itsRawValues);
    auto minBigValue = ::GetMinBigValue(itsRawValues);
    if (maxSmallValue != kFloatMissing && minBigValue != kFloatMissing)
    {
      auto modSpread = maxSmallValue - (minBigValue - 360);
      if (modSpread <= 180)
      {
        itsMinValue = minBigValue;
        itsMaxValue = maxSmallValue;
      }
    }
  }
}

void NFmiDataModifierModMinMax::DoSomeTestRoutines()
{
  // Testi 20, 50, 90, 120  ==>  min 20 ja max 120
  auto values = std::vector<float>{20, 50, 90, 120};
  auto results = std::make_pair(20.f, 120.f);
  ::TestModMinMax(values, results);

  values = std::vector<float>{20, 50, 90, 340};
  results = std::make_pair(340.f, 90.f);
  ::TestModMinMax(values, results);

  values = std::vector<float>{};
  results = std::make_pair(kFloatMissing, kFloatMissing);
  ::TestModMinMax(values, results);

  values = std::vector<float>{210};
  results = std::make_pair(210.f, 210.f);
  ::TestModMinMax(values, results);

  values = std::vector<float>{20, 150, 240, 340};
  results = std::make_pair(20.f, 340.f);
  ::TestModMinMax(values, results);
}
