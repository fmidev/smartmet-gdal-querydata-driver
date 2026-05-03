#include "NFmiDataModifierModAvg.h"

namespace
{
void TestModAvg(const std::vector<float>& values, float expected)
{
  NFmiDataModifierModAvg modAvg;
  for (auto value : values)
  {
    modAvg.Calculate(value);
  }
  auto avg = modAvg.CalculationResult();
  if (avg != expected)
  {
    std::string errorMessage = "NFmiDataModifierModAvg tester failed: value ";
    errorMessage += std::to_string(avg);
    errorMessage += " was not expected ";
    errorMessage += std::to_string(expected);
    throw std::runtime_error(errorMessage);
  }
}

}  // namespace

NFmiDataModifierModAvg::~NFmiDataModifierModAvg() = default;
NFmiDataModifierModAvg::NFmiDataModifierModAvg() = default;
NFmiDataModifierModAvg::NFmiDataModifierModAvg(const NFmiDataModifierModAvg& theOther) = default;

NFmiDataModifierModAvg* NFmiDataModifierModAvg::Clone() const
{
  return new NFmiDataModifierModAvg(*this);
}

void NFmiDataModifierModAvg::Clear() { itsModMeanCalculator = NFmiModMeanCalculator(); }

void NFmiDataModifierModAvg::Calculate(float theValue)
{
  if (theValue != kFloatMissing)
  {
    itsModMeanCalculator(theValue);
  }
}
float NFmiDataModifierModAvg::CalculationResult() { return itsModMeanCalculator(); }

void NFmiDataModifierModAvg::DoSomeTestRoutines()
{
  // Testi 20, 50  ==>  avg = 35
  auto values = std::vector<float>{20, 50};
  auto result = 35.f;
  ::TestModAvg(values, result);

  values = std::vector<float>{10, 340};
  result = 355.f;
  ::TestModAvg(values, result);
}
