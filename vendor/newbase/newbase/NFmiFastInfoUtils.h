#pragma once

#include "NFmiDataIdent.h"
#include "NFmiDataMatrix.h"
#include "NFmiMetTime.h"

#include <deque>
#include <memory>

class NFmiFastQueryInfo;
class NFmiLevel;
class NFmiPoint;
class NFmiQueryInfo;

namespace NFmiFastInfoUtils
{
// Apu luokka tekem‰‰n ik‰v‰n fastInfon parametrin valinta tilan palautuksen (indeksi + aliparametri
// juttu). Tila otetaan konstruktorissa ja palautetaan destruktorissa.
class QueryInfoParamStateRestorer
{
 protected:
  NFmiQueryInfo &info_;
  FmiParameterName paramId_;

 public:
  QueryInfoParamStateRestorer(NFmiQueryInfo &info);
  virtual ~QueryInfoParamStateRestorer();
};

class QueryInfoTotalStateRestorer : public QueryInfoParamStateRestorer
{
  unsigned long locationIndex_;
  unsigned long timeIndex_;
  unsigned long levelIndex_;

 public:
  QueryInfoTotalStateRestorer(NFmiQueryInfo &info);
  ~QueryInfoTotalStateRestorer();
};

class MetaWindParamUsage
{
  // If Totalwind combine parameter is present, no need for any meta parameters...
  bool fHasTotalWind = false;
  // Data has kFmiWindVectorMS parameter, so no need to make meta parameter of it.
  bool fHasWindVectorParam = false;
  // Data has wind speed and wind direction (that can be used to calculate wind vector)
  bool fHasWsAndWd = false;
  // data has u- and v-components of the wind (that can be used to calculate wind vector)
  bool fHasWindComponents = false;

 public:
  MetaWindParamUsage();
  MetaWindParamUsage(bool hasTotalWind,
                     bool hasWindVectorParam,
                     bool hasWsAndWd,
                     bool hasWindComponents);

  bool ParamNeedsMetaCalculations(unsigned long paramId) const;
  bool NoWindMetaParamsNeeded() const;
  bool MakeMetaWindVectorParam() const;
  bool MakeMetaWsAndWdParams() const;
  bool MakeMetaWindComponents() const;
  bool HasTotalWind() const { return fHasTotalWind; }
  bool HasWindVectorParam() const { return fHasWindVectorParam; }
  bool HasWsAndWd() const { return fHasWsAndWd; }
  bool HasWindComponents() const { return fHasWindComponents; }
  bool IsStreamlinePossible() const;

  friend MetaWindParamUsage CheckMetaWindParamUsage(NFmiQueryInfo &theInfo);
};

bool IsInfoShipTypeData(NFmiFastQueryInfo &theInfo);
void SetSoundingDataLevel(const NFmiLevel &theWantedSoundingPressureLevel, NFmiFastQueryInfo &info);
std::string GetTotalDataFilePath(const std::shared_ptr<NFmiFastQueryInfo> &theInfo);
bool IsYearLongData(const std::shared_ptr<NFmiFastQueryInfo> &info);
bool IsModelClimatologyData(const std::shared_ptr<NFmiFastQueryInfo> &info);
NFmiMetTime GetUsedTimeIfModelClimatologyData(std::shared_ptr<NFmiFastQueryInfo> &theInfo,
                                              const NFmiMetTime &theTime);
bool IsMovingSoundingData(const std::shared_ptr<NFmiFastQueryInfo> &theInfo);
bool IsLightningTypeData(std::shared_ptr<NFmiFastQueryInfo> &info);
bool FindTimeIndicesForGivenTimeRange(const std::shared_ptr<NFmiFastQueryInfo> &theInfo,
                                      const NFmiMetTime &theStartTime,
                                      long minuteRange,
                                      unsigned long &timeIndex1,
                                      unsigned long &timeIndex2);
bool FindMovingSoundingDataTime(const std::shared_ptr<NFmiFastQueryInfo> &theInfo,
                                const NFmiMetTime &theTime,
                                NFmiLocation &theLocation);
MetaWindParamUsage CheckMetaWindParamUsage(const std::shared_ptr<NFmiFastQueryInfo> &theInfo);
MetaWindParamUsage CheckMetaWindParamUsage(NFmiQueryInfo &theInfo);
std::vector<std::unique_ptr<NFmiDataIdent>> MakePossibleWindMetaParams(
    NFmiQueryInfo &theInfo, bool allowStreamlineParameter);
float GetMetaWindValue(const std::shared_ptr<NFmiFastQueryInfo> &theInfo,
                       const MetaWindParamUsage &metaWindParamUsage,
                       unsigned long wantedParamId);
float GetMetaWindValue(const std::shared_ptr<NFmiFastQueryInfo> &theInfo,
                       const NFmiMetTime &theTime,
                       const MetaWindParamUsage &metaWindParamUsage,
                       unsigned long wantedParamId);
float GetMetaWindValue(const std::shared_ptr<NFmiFastQueryInfo> &theInfo,
                       const NFmiPoint &theLatlon,
                       const MetaWindParamUsage &metaWindParamUsage,
                       unsigned long wantedParamId);
float GetMetaWindValue(const std::shared_ptr<NFmiFastQueryInfo> &theInfo,
                       const NFmiMetTime &theTime,
                       const NFmiPoint &theLatlon,
                       const MetaWindParamUsage &metaWindParamUsage,
                       unsigned long wantedParamId);
void CalcDequeWindSpeedAndDirectionFromComponents(const std::deque<float> &u,
                                                  const std::deque<float> &v,
                                                  std::deque<float> &wsOut,
                                                  std::deque<float> &wdOut);
void CalcDequeWindComponentsFromSpeedAndDirection(const std::deque<float> &ws,
                                                  const std::deque<float> &wd,
                                                  std::deque<float> &uOut,
                                                  std::deque<float> &vOut);
void CalcMatrixWindComponentsFromSpeedAndDirection(const NFmiDataMatrix<float> &ws,
                                                   const NFmiDataMatrix<float> &wd,
                                                   NFmiDataMatrix<float> &uOut,
                                                   NFmiDataMatrix<float> &vOut);
void CalcDequeWindVectorFromSpeedAndDirection(const std::deque<float> &ws,
                                              const std::deque<float> &wd,
                                              std::deque<float> &windVectorOut);
void CalcMatrixWindVectorFromSpeedAndDirection(const NFmiDataMatrix<float> &ws,
                                               const NFmiDataMatrix<float> &wd,
                                               NFmiDataMatrix<float> &windVectorOut,
                                               unsigned int theStartColumnIndex = 0);
void CalcMatrixWindVectorFromWindComponents(const NFmiDataMatrix<float> &u,
                                            const NFmiDataMatrix<float> &v,
                                            NFmiDataMatrix<float> &windVectorOut,
                                            unsigned int theStartColumnIndex = 0);
void CalcMatrixWsFromWindComponents(const NFmiDataMatrix<float> &u,
                                    const NFmiDataMatrix<float> &v,
                                    NFmiDataMatrix<float> &wsOut,
                                    unsigned int theStartColumnIndex = 0);
void CalcMatrixWdFromWindComponents(const NFmiDataMatrix<float> &u,
                                    const NFmiDataMatrix<float> &v,
                                    NFmiDataMatrix<float> &wdOut,
                                    unsigned int theStartColumnIndex = 0);
void CalcMatrixUcomponentFromSpeedAndDirection(const NFmiDataMatrix<float> &ws,
                                               const NFmiDataMatrix<float> &wd,
                                               NFmiDataMatrix<float> &uOut,
                                               unsigned int theStartColumnIndex = 0);
void CalcMatrixVcomponentFromSpeedAndDirection(const NFmiDataMatrix<float> &ws,
                                               const NFmiDataMatrix<float> &wd,
                                               NFmiDataMatrix<float> &vOut,
                                               unsigned int theStartColumnIndex = 0);
bool SetInfoToGridPoint(std::shared_ptr<NFmiFastQueryInfo> &info,
                        unsigned long gridPointX,
                        unsigned long gridPointY);

float CalcWS(float u, float v);
float CalcWD(float u, float v);
float CalcU(float WS, float WD);
float CalcV(float WS, float WD);
float CalcWindVectorFromWindComponents(float u, float v);
float CalcWindVectorFromSpeedAndDirection(float WS, float WD);
}  // namespace NFmiFastInfoUtils
