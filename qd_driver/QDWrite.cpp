#include "QDWrite.h"
#include "QDStrings.h"

#include <newbase/NFmiArea.h>
#include <newbase/NFmiDataIdent.h>
#include <newbase/NFmiDataMatrix.h>
#include <newbase/NFmiFastQueryInfo.h>
#include <newbase/NFmiGlobals.h>
#include <newbase/NFmiGrid.h>
#include <newbase/NFmiHPlaceDescriptor.h>
#include <newbase/NFmiLevel.h>
#include <newbase/NFmiLevelBag.h>
#include <newbase/NFmiLevelType.h>
#include <newbase/NFmiMetTime.h>
#include <newbase/NFmiParam.h>
#include <newbase/NFmiParamBag.h>
#include <newbase/NFmiParamDescriptor.h>
#include <newbase/NFmiParameterName.h>
#include <newbase/NFmiQueryData.h>
#include <newbase/NFmiQueryDataUtil.h>
#include <newbase/NFmiQueryInfo.h>
#include <newbase/NFmiTimeBag.h>
#include <newbase/NFmiTimeDescriptor.h>
#include <newbase/NFmiTimeList.h>
#include <newbase/NFmiVPlaceDescriptor.h>

#include <gis/SpatialReference.h>

#include <cpl_conv.h>
#include <cpl_error.h>
#include <cpl_string.h>

#include <ogr_spatialref.h>

#include <fmt/format.h>

#include <cstdlib>
#include <ctime>
#include <exception>
#include <memory>
#include <string>
#include <vector>

namespace SmartMet
{
namespace GdalQueryData
{
namespace
{
// Parse an ISO-8601-ish UTC timestamp ("YYYY-MM-DDTHH:MM:SS[Z]" or just date)
// into an NFmiMetTime. Returns false if unparseable.
bool parseIsoTime(const char* s, NFmiMetTime& out)
{
  if (s == nullptr) return false;
  int Y = 0, M = 0, D = 0, h = 0, m = 0, sec = 0;
  if (std::sscanf(s, "%d-%d-%dT%d:%d:%d", &Y, &M, &D, &h, &m, &sec) >= 5)
  {
    out = NFmiMetTime(static_cast<short>(Y), static_cast<short>(M), static_cast<short>(D),
                      static_cast<short>(h), static_cast<short>(m), 0);
    return true;
  }
  if (std::sscanf(s, "%d-%d-%d", &Y, &M, &D) == 3)
  {
    out = NFmiMetTime(static_cast<short>(Y), static_cast<short>(M), static_cast<short>(D));
    return true;
  }
  return false;
}

// Pick the value for an option from (in priority order): creation options,
// source dataset metadata, source band 1 metadata, fallback.
const char* fetchOpt(CSLConstList options, GDALDataset* src, const char* key)
{
  if (options != nullptr)
  {
    if (const char* v = CSLFetchNameValue(options, key)) return v;
  }
  if (src != nullptr)
  {
    if (const char* v = src->GetMetadataItem(key)) return v;
    if (src->GetRasterCount() > 0)
    {
      auto* b = src->GetRasterBand(1);
      if (b != nullptr)
        if (const char* v = b->GetMetadataItem(key)) return v;
    }
  }
  return nullptr;
}

}  // namespace

GDALDataset* createCopy(const char* pszFilename, GDALDataset* poSrcDS, int /*bStrict*/,
                        char** papszOptions, GDALProgressFunc pfnProgress, void* pProgressData)
{
  if (poSrcDS == nullptr || pszFilename == nullptr)
  {
    CPLError(CE_Failure, CPLE_AppDefined, "querydata CreateCopy: missing source or filename");
    return nullptr;
  }

  const int nx = poSrcDS->GetRasterXSize();
  const int ny = poSrcDS->GetRasterYSize();
  const int nBands = poSrcDS->GetRasterCount();
  if (nx <= 0 || ny <= 0 || nBands <= 0)
  {
    CPLError(CE_Failure, CPLE_AppDefined,
             "querydata CreateCopy: source must have non-empty raster and at least one band");
    return nullptr;
  }

  GDALGeoTransform gt;
  if (poSrcDS->GetGeoTransform(gt) != CE_None)
  {
    CPLError(CE_Failure, CPLE_AppDefined, "querydata CreateCopy: source has no geotransform");
    return nullptr;
  }
  if (gt.xrot != 0.0 || gt.yrot != 0.0)
  {
    CPLError(CE_Failure, CPLE_NotSupported,
             "querydata CreateCopy: rotated geotransforms are not supported "
             "(rot=%g,%g); reproject the source first",
             gt.xrot, gt.yrot);
    return nullptr;
  }

  const auto* srcSrs = poSrcDS->GetSpatialRef();
  if (srcSrs == nullptr)
  {
    CPLError(CE_Failure, CPLE_AppDefined,
             "querydata CreateCopy: source has no spatial reference");
    return nullptr;
  }

  // Projected-coordinate corners. With a typical top-down geotransform
  // (negative yscale) the upper-left is (xorig, yorig) and the lower-right is
  // (xorig + nx*xscale, yorig + ny*yscale).
  const double xLeft = gt.xorig;
  const double xRight = gt.xorig + gt.xscale * nx;
  const double yTop = gt.yorig;
  const double yBottom = gt.yorig + gt.yscale * ny;
  const double xMin = std::min(xLeft, xRight);
  const double xMax = std::max(xLeft, xRight);
  const double yMin = std::min(yTop, yBottom);
  const double yMax = std::max(yTop, yBottom);

  // NFmiArea::CreateFromBBox uses DetectClassId internally to pick a native
  // newbase projection (LatLon/Mercator/Stereographic/Lambert/...) when the
  // source projection matches a known family, otherwise wraps via NFmiGdalArea.
  // We delegate the choice rather than re-implementing it here.
  std::unique_ptr<NFmiArea> area;
  try
  {
    Fmi::SpatialReference sr(*srcSrs);
    area.reset(NFmiArea::CreateFromBBox(sr, NFmiPoint(xMin, yMin), NFmiPoint(xMax, yMax)));
  }
  catch (const std::exception& e)
  {
    CPLError(CE_Failure, CPLE_NotSupported,
             "querydata CreateCopy: cannot map source SRS to an NFmiArea: %s", e.what());
    return nullptr;
  }
  if (!area)
  {
    CPLError(CE_Failure, CPLE_NotSupported,
             "querydata CreateCopy: source SRS produced no usable NFmiArea");
    return nullptr;
  }

  // QD grid storage: kBottomLeft is the conventional starting corner. We
  // convert top-down GDAL rows to bottom-up QD rows on write.
  //
  // NOTE: must use the (Grid)-only constructor, not (Area, Grid). The latter
  // sets both itsArea and itsGrid; NFmiHPlaceDescriptor::Size() then takes the
  // IsArea() branch first and reports 1 location, which makes SetValues poke
  // exactly one cell. The Grid-only constructor leaves itsArea null and Size()
  // correctly returns nx*ny.
  NFmiGrid grid(area.get(), static_cast<unsigned long>(nx), static_cast<unsigned long>(ny),
                kBottomLeft);
  NFmiHPlaceDescriptor hPlace(grid);

  // Parameter
  long paramId = 4;  // kFmiTemperature
  std::string paramName = "Data";
  if (const char* v = fetchOpt(papszOptions, poSrcDS, "PARAM_ID")) paramId = std::atol(v);
  if (const char* v = fetchOpt(papszOptions, poSrcDS, "PARAM_NAME")) paramName = v;
  // newbase strings are Latin-1; convert if the caller passed UTF-8 (the
  // typical case from a NetCDF/GeoTIFF source).
  paramName = fromUtf8(paramName);
  NFmiParam param(static_cast<unsigned long>(paramId), paramName.c_str());
  NFmiParamBag paramBag;
  paramBag.Add(NFmiDataIdent(param));
  NFmiParamDescriptor paramDesc(paramBag);

  // Single level (creation options can override).
  float levelValue = 0.f;
  long levelType = 1;  // ground / surface
  if (const char* v = fetchOpt(papszOptions, poSrcDS, "LEVEL_VALUE")) levelValue = std::atof(v);
  if (const char* v = fetchOpt(papszOptions, poSrcDS, "LEVEL_TYPE")) levelType = std::atol(v);
  NFmiLevelBag levelBag(static_cast<FmiLevelType>(levelType), levelValue, levelValue, 0.f);
  NFmiVPlaceDescriptor vPlace(levelBag);

  // Times: read VALID_TIME per band; if any band is missing it, fall back to
  // START_TIME + TIMESTEP_MINUTES creation options; if those are missing too,
  // synthesise hourly steps from epoch as a last resort.
  NFmiMetTime originTime;  // default = current UTC
  bool haveOrigin = false;
  if (const char* v = fetchOpt(papszOptions, poSrcDS, "ORIGIN_TIME"))
    haveOrigin = parseIsoTime(v, originTime);
  // Origin defaults to "now" but NFmiMetTime() that's fine.

  std::vector<NFmiMetTime> validTimes;
  validTimes.reserve(nBands);
  bool allBandTimes = true;
  for (int b = 1; b <= nBands; ++b)
  {
    auto* band = poSrcDS->GetRasterBand(b);
    NFmiMetTime t;
    if (band == nullptr || !parseIsoTime(band->GetMetadataItem("VALID_TIME"), t))
    {
      allBandTimes = false;
      break;
    }
    validTimes.push_back(t);
  }

  if (!allBandTimes)
  {
    validTimes.clear();
    NFmiMetTime startTime;
    long stepMinutes = 60;
    if (const char* v = fetchOpt(papszOptions, poSrcDS, "START_TIME"))
    {
      if (!parseIsoTime(v, startTime))
      {
        CPLError(CE_Failure, CPLE_AppDefined, "querydata CreateCopy: invalid START_TIME '%s'", v);
        return nullptr;
      }
    }
    if (const char* v = fetchOpt(papszOptions, poSrcDS, "TIMESTEP_MINUTES"))
      stepMinutes = std::atol(v);
    for (int b = 0; b < nBands; ++b)
    {
      NFmiMetTime t = startTime;
      t.ChangeByMinutes(b * stepMinutes);
      validTimes.push_back(t);
    }
  }

  if (!haveOrigin && !validTimes.empty()) originTime = validTimes.front();

  NFmiTimeList timeList;
  for (auto& t : validTimes) timeList.Add(new NFmiMetTime(t));
  NFmiTimeDescriptor timeDesc(originTime, timeList);

  // Build the QueryInfo + QueryData. Use CreateEmptyData rather than the
  // NFmiQueryData(info) constructor: the latter only copies the descriptors
  // and leaves the raw data pool unallocated (writing then produces a 0-byte
  // file with "Invalid datapool size" on read-back). CreateEmptyData allocates
  // a fresh kFloatMissing-filled pool of the right size.
  std::unique_ptr<NFmiQueryData> data;
  try
  {
    NFmiQueryInfo info(paramDesc, timeDesc, hPlace, vPlace, 7.0);
    data.reset(NFmiQueryDataUtil::CreateEmptyData(info));
  }
  catch (const std::exception& e)
  {
    CPLError(CE_Failure, CPLE_AppDefined,
             "querydata CreateCopy: failed to construct QueryData: %s", e.what());
    return nullptr;
  }
  if (!data)
  {
    CPLError(CE_Failure, CPLE_AppDefined,
             "querydata CreateCopy: NFmiQueryDataUtil::CreateEmptyData returned null");
    return nullptr;
  }

  NFmiFastQueryInfo fi(data.get());
  fi.First();
  if (!fi.ParamIndex(0) || !fi.LevelIndex(0))
  {
    CPLError(CE_Failure, CPLE_AppDefined, "querydata CreateCopy: failed to position iterator");
    return nullptr;
  }

  std::vector<float> rowMajor(static_cast<size_t>(nx) * static_cast<size_t>(ny));
  for (int b = 1; b <= nBands; ++b)
  {
    auto* band = poSrcDS->GetRasterBand(b);
    const CPLErr ioErr = band->RasterIO(GF_Read, 0, 0, nx, ny, rowMajor.data(), nx, ny,
                                        GDT_Float32, 0, 0, nullptr);
    if (ioErr != CE_None)
    {
      CPLError(CE_Failure, CPLE_AppDefined,
               "querydata CreateCopy: failed to read source band %d", b);
      return nullptr;
    }

    // Map source band's nodata to kFloatMissing so QD readers see "missing"
    // consistently. If the source has no nodata, leave values untouched.
    int hasNoData = 0;
    const double srcNoData = band->GetNoDataValue(&hasNoData);
    if (hasNoData)
    {
      const float srcNoDataF = static_cast<float>(srcNoData);
      for (auto& v : rowMajor)
        if (v == srcNoDataF) v = kFloatMissing;
    }

    NFmiDataMatrix<float> matrix(static_cast<unsigned long>(nx),
                                 static_cast<unsigned long>(ny), kFloatMissing);
    for (int row = 0; row < ny; ++row)
    {
      const int qdRow = ny - 1 - row;  // GDAL top-down → QD bottom-up
      for (int col = 0; col < nx; ++col)
        matrix[col][qdRow] = rowMajor[static_cast<size_t>(row) * nx + col];
    }

    if (!fi.TimeIndex(static_cast<unsigned long>(b - 1)))
    {
      CPLError(CE_Failure, CPLE_AppDefined,
               "querydata CreateCopy: time index %d out of range", b - 1);
      return nullptr;
    }
    if (!fi.SetValues(matrix))
    {
      CPLError(CE_Failure, CPLE_AppDefined,
               "querydata CreateCopy: SetValues failed for band %d", b);
      return nullptr;
    }

    if (pfnProgress != nullptr)
    {
      const double ratio = static_cast<double>(b) / static_cast<double>(nBands);
      if (!pfnProgress(ratio, nullptr, pProgressData))
      {
        CPLError(CE_Failure, CPLE_UserInterrupt, "querydata CreateCopy: cancelled");
        return nullptr;
      }
    }
  }

  try
  {
    // NFmiQueryData has Write(bool) and Write(const std::string&) overloads.
    // const char* implicitly converts to bool, so we must build a std::string
    // explicitly or the file would be dumped to stdout.
    data->Write(std::string(pszFilename));
  }
  catch (const std::exception& e)
  {
    CPLError(CE_Failure, CPLE_OpenFailed,
             "querydata CreateCopy: write to '%s' failed: %s", pszFilename, e.what());
    return nullptr;
  }

  // Reopen the freshly-written file so the caller gets a normal read handle.
  return GDALDataset::Open(pszFilename, GDAL_OF_RASTER | GDAL_OF_READONLY);
}

}  // namespace GdalQueryData
}  // namespace SmartMet
