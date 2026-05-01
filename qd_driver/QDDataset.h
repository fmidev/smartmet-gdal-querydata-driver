#pragma once

#include <gdal_dataset.h>
#include <gdal_geotransform.h>

#include <memory>
#include <mutex>
#include <string>

class NFmiQueryData;
class NFmiFastQueryInfo;
class OGRSpatialReference;
class GDALGroup;

namespace SmartMet
{
namespace GdalQueryData
{
// One QDDataset corresponds either to:
//   - the top-level view of a .sqd file (subdatasets, no bands), or
//   - a single (paramIdx, levelIdx) slice of a .sqd file (bands = time steps).
class QDDataset : public GDALDataset
{
 public:
  ~QDDataset() override;

  static int Identify(GDALOpenInfo* poOpenInfo);
  static GDALDataset* Open(GDALOpenInfo* poOpenInfo);

  CPLErr GetGeoTransform(GDALGeoTransform& gt) const override;
  const OGRSpatialReference* GetSpatialRef() const override;

  std::shared_ptr<GDALGroup> GetRootGroup() const override { return itsRootGroup; }

  // Used by QDRasterBand. The mutex must be held while changing
  // iterator state (param/level/time index) and reading.
  NFmiFastQueryInfo& info() { return *itsInfo; }
  std::mutex& mutex() { return itsMutex; }

  unsigned long paramIndex() const { return itsParamIndex; }
  unsigned long levelIndex() const { return itsLevelIndex; }
  unsigned long gridYSize() const { return itsGridYSize; }

  // 0 means "no sub-param activation"; non-zero is an FmiParameterName value
  // identifying a sub-parameter of a combined parameter (TotalWind,
  // WeatherAndCloudiness). When set, IReadBlock activates it via
  // Param(FmiParameterName) instead of ParamIndex.
  unsigned long subParamId() const { return itsSubParamId; }

  QDDataset();

 private:

  // Open path "FILE.sqd" (top-level) or
  // "querydata:\"FILE.sqd\":P:L[:S]" where S is an optional sub-parameter
  // FmiParameterName id (e.g. kFmiWindSpeedMS).
  static GDALDataset* openTopLevel(const std::string& path);
  static GDALDataset* openSubdataset(const std::string& path,
                                     unsigned long paramIdx,
                                     unsigned long levelIdx,
                                     unsigned long subParamId);

  void buildSubdatasetList();
  void buildBands();
  void buildGeoTransformAndSRS();

  std::shared_ptr<NFmiQueryData> itsData;
  std::unique_ptr<NFmiFastQueryInfo> itsInfo;
  std::mutex itsMutex;

  std::string itsSourcePath;

  // Only meaningful when we are a single-slice view (bands exposed):
  unsigned long itsParamIndex = 0;
  unsigned long itsLevelIndex = 0;
  unsigned long itsGridXSize = 0;
  unsigned long itsGridYSize = 0;
  unsigned long itsSubParamId = 0;  // 0 = no sub-param; otherwise FmiParameterName

  GDALGeoTransform itsGeoTransform;
  bool itsHasGeoTransform = false;

  std::unique_ptr<OGRSpatialReference> itsSRS;
  std::shared_ptr<GDALGroup> itsRootGroup;
};

}  // namespace GdalQueryData
}  // namespace SmartMet
