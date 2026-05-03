#include "NFmiWindFix.h"
#include "NFmiFastInfoUtils.h"
#include "NFmiFastQueryInfo.h"
#include "NFmiQueryData.h"
#include <macgyver/Exception.h>

using namespace std;

namespace
{
bool RecalculateWindParameters(NFmiFastQueryInfo& sourceInfo,
                               NFmiFastQueryInfo& destInfo,
                               const NFmiFastInfoUtils::MetaWindParamUsage& metaWindParamUsage)
{
  try
  {
    sourceInfo.Param(kFmiWindSpeedMS);
    auto wsParamIndex = sourceInfo.ParamIndex();
    sourceInfo.Param(kFmiWindDirection);
    auto wdParamIndex = sourceInfo.ParamIndex();
    sourceInfo.Param(kFmiWindUMS);
    auto uParamIndex = sourceInfo.ParamIndex();
    sourceInfo.Param(kFmiWindVMS);
    auto vParamIndex = sourceInfo.ParamIndex();
    sourceInfo.Param(kFmiWindVectorMS);
    auto windVectorParamIndex = sourceInfo.ParamIndex();
    for (sourceInfo.ResetLevel(), destInfo.ResetLevel();
         sourceInfo.NextLevel() && destInfo.NextLevel();)
    {
      for (sourceInfo.ResetLocation(), destInfo.ResetLocation();
           sourceInfo.NextLocation() && destInfo.NextLocation();)
      {
        for (sourceInfo.ResetTime(), destInfo.ResetTime();
             sourceInfo.NextTime() && destInfo.NextTime();)
        {
          sourceInfo.ParamIndex(wsParamIndex);
          auto WS = sourceInfo.FloatValue();
          sourceInfo.ParamIndex(wdParamIndex);
          auto WD = sourceInfo.FloatValue();
          if (destInfo.ParamIndex(uParamIndex))
            destInfo.FloatValue(NFmiFastInfoUtils::CalcU(WS, WD));
          if (destInfo.ParamIndex(vParamIndex))
            destInfo.FloatValue(NFmiFastInfoUtils::CalcV(WS, WD));
          if (destInfo.ParamIndex(windVectorParamIndex))
            destInfo.FloatValue(NFmiFastInfoUtils::CalcWindVectorFromSpeedAndDirection(WS, WD));
        }
      }
    }
    return true;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

}  // namespace

namespace NFmiWindFix
{
bool FixWinds(NFmiQueryData& sourceData)
{
  try
  {
    NFmiFastQueryInfo sourceInfo(&sourceData);
    return FixWinds(sourceInfo);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

bool FixWinds(NFmiFastQueryInfo& sourceInfo)
{
  try
  {
    NFmiFastInfoUtils::MetaWindParamUsage metaWindParamUsage =
        NFmiFastInfoUtils::CheckMetaWindParamUsage(sourceInfo);
    if (metaWindParamUsage.HasTotalWind())
      return false;
    if (!metaWindParamUsage.HasWsAndWd())
      return false;

    if (metaWindParamUsage.HasWindComponents() || metaWindParamUsage.HasWindVectorParam())
    {
      NFmiFastQueryInfo destInfo(sourceInfo);
      return ::RecalculateWindParameters(sourceInfo, destInfo, metaWindParamUsage);
    }
    return false;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

}  // namespace NFmiWindFix
