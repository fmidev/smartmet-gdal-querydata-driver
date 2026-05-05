#include "QDDataset.h"
#include "QDMultidim.h"
#include "QDRasterBand.h"
#include "QDStrings.h"

#include <algorithm>
#include <newbase/NFmiArea.h>
#include <newbase/NFmiDataIdent.h>
#include <newbase/NFmiFastQueryInfo.h>
#include <newbase/NFmiGlobals.h>
#include <newbase/NFmiGrid.h>
#include <newbase/NFmiLevel.h>
#include <newbase/NFmiMetTime.h>
#include <newbase/NFmiParam.h>
#include <newbase/NFmiParamBag.h>
#include <newbase/NFmiQueryData.h>
#include <newbase/NFmiRect.h>

#include <gdal_driver.h>
#include <gdal_drivermanager.h>
#include <gdal_priv.h>

#include <ogr_spatialref.h>

#include <cpl_conv.h>
#include <cpl_error.h>
#include <cpl_string.h>

#include <fmt/format.h>

#include <cstring>
#include <exception>
#include <vector>

namespace SmartMet
{
namespace GdalQueryData
{
namespace
{
constexpr const char* kDriverName = "querydata";
constexpr const char* kSubdatasetPrefix = "querydata:";

// First 9 bytes of every .sqd file: '@', '$', 0xb0, 0xa3, 'Q', 'I', 'N', 'F', 'O'.
const unsigned char kMagic[9] = {'@', '$', 0xb0, 0xa3, 'Q', 'I', 'N', 'F', 'O'};

bool hasMagic(const GByte* buf, int n)
{
  if (n < static_cast<int>(sizeof(kMagic))) return false;
  return std::memcmp(buf, kMagic, sizeof(kMagic)) == 0;
}

// Parse "querydata:\"path\":P:L[:S]" into (path, paramIdx, levelIdx, subParamId).
// subParamId is 0 if absent. Returns false if the spec is malformed.
bool parseSubdatasetSpec(const std::string& spec,
                         std::string& path,
                         unsigned long& paramIdx,
                         unsigned long& levelIdx,
                         unsigned long& subParamId)
{
  subParamId = 0;
  if (spec.rfind(kSubdatasetPrefix, 0) != 0) return false;
  size_t i = std::strlen(kSubdatasetPrefix);
  if (i >= spec.size()) return false;

  // Path: either quoted ("...") or up to next ':'
  if (spec[i] == '"')
  {
    size_t end = spec.find('"', i + 1);
    if (end == std::string::npos) return false;
    path = spec.substr(i + 1, end - i - 1);
    i = end + 1;
  }
  else
  {
    size_t end = spec.find(':', i);
    if (end == std::string::npos) return false;
    path = spec.substr(i, end - i);
    i = end;
  }

  if (i >= spec.size() || spec[i] != ':') return false;
  ++i;

  size_t firstColon = spec.find(':', i);
  if (firstColon == std::string::npos) return false;
  try
  {
    paramIdx = std::stoul(spec.substr(i, firstColon - i));
    size_t secondColon = spec.find(':', firstColon + 1);
    if (secondColon == std::string::npos)
    {
      levelIdx = std::stoul(spec.substr(firstColon + 1));
    }
    else
    {
      levelIdx = std::stoul(spec.substr(firstColon + 1, secondColon - firstColon - 1));
      subParamId = std::stoul(spec.substr(secondColon + 1));
    }
  }
  catch (...)
  {
    return false;
  }
  return true;
}

std::string formatSubdatasetSpec(const std::string& path,
                                 unsigned long p,
                                 unsigned long l,
                                 unsigned long subParamId = 0)
{
  if (subParamId == 0)
    return fmt::format("{}\"{}\":{}:{}", kSubdatasetPrefix, path, p, l);
  return fmt::format("{}\"{}\":{}:{}:{}", kSubdatasetPrefix, path, p, l, subParamId);
}

// Pretty time tag for band metadata.
std::string formatTime(const NFmiMetTime& t)
{
  return fmt::format("{:04d}-{:02d}-{:02d}T{:02d}:{:02d}:00Z",
                     t.GetYear(),
                     t.GetMonth(),
                     t.GetDay(),
                     t.GetHour(),
                     t.GetMin());
}

}  // namespace

QDDataset::QDDataset() = default;
QDDataset::~QDDataset() = default;

int QDDataset::Identify(GDALOpenInfo* poOpenInfo)
{
  // Subdataset spec
  if (poOpenInfo->pszFilename != nullptr &&
      std::strncmp(poOpenInfo->pszFilename, kSubdatasetPrefix, std::strlen(kSubdatasetPrefix)) == 0)
    return TRUE;

  // File path: check magic in header buffer GDAL preloaded
  if (poOpenInfo->fpL != nullptr && poOpenInfo->nHeaderBytes >= 9 &&
      hasMagic(poOpenInfo->pabyHeader, poOpenInfo->nHeaderBytes))
    return TRUE;

  return FALSE;
}

GDALDataset* QDDataset::Open(GDALOpenInfo* poOpenInfo)
{
  if (!Identify(poOpenInfo)) return nullptr;
  if (poOpenInfo->eAccess == GA_Update)
  {
    CPLError(CE_Failure, CPLE_NotSupported, "querydata driver is read-only");
    return nullptr;
  }

  const std::string spec = poOpenInfo->pszFilename;

  std::string path;
  unsigned long paramIdx = 0;
  unsigned long levelIdx = 0;
  unsigned long subParamId = 0;
  if (parseSubdatasetSpec(spec, path, paramIdx, levelIdx, subParamId))
    return openSubdataset(path, paramIdx, levelIdx, subParamId);

  return openTopLevel(spec);
}

GDALDataset* QDDataset::openTopLevel(const std::string& path)
{
  std::shared_ptr<NFmiQueryData> data;
  try
  {
    data = std::make_shared<NFmiQueryData>(path, /*memoryMap=*/true);
  }
  catch (const std::exception& e)
  {
    CPLError(CE_Failure, CPLE_OpenFailed, "Failed to open '%s': %s", path.c_str(), e.what());
    return nullptr;
  }

  auto info = std::make_unique<NFmiFastQueryInfo>(data.get());
  if (!info->IsGrid())
  {
    CPLError(CE_Failure, CPLE_NotSupported,
             "querydata driver only supports gridded data; '%s' uses station locations",
             path.c_str());
    return nullptr;
  }

  const unsigned long nParams = info->SizeParams();
  const unsigned long nLevels = info->SizeLevels();

  // Special case: a single (param, level), no sub-params — expose bands directly so
  // users do not need to know about subdatasets for the common forecast-of-one-quantity
  // file. We still go through the subdataset listing path if the single param has
  // sub-params (e.g. a wind-only file has TotalWind with multiple sub-params).
  if (nParams == 1 && nLevels == 1)
  {
    info->ParamIndex(0);
    if (!info->Param().HasDataParams()) return openSubdataset(path, 0, 0, 0);
  }

  auto ds = std::make_unique<QDDataset>();
  ds->itsData = data;
  ds->itsInfo = std::move(info);
  ds->itsSourcePath = path;
  // Top-level dataset: 0 bands, only a subdataset listing in the classic API.
  // The MDR root group exposes the same data as a proper N-D view for xarray.
  ds->nRasterXSize = 0;
  ds->nRasterYSize = 0;
  ds->buildSubdatasetList();
  ds->itsRootGroup = buildRootGroup(*ds);
  ds->SetDescription(path.c_str());
  return ds.release();
}

GDALDataset* QDDataset::openSubdataset(const std::string& path,
                                       unsigned long paramIdx,
                                       unsigned long levelIdx,
                                       unsigned long subParamId)
{
  std::shared_ptr<NFmiQueryData> data;
  try
  {
    data = std::make_shared<NFmiQueryData>(path, /*memoryMap=*/true);
  }
  catch (const std::exception& e)
  {
    CPLError(CE_Failure, CPLE_OpenFailed, "Failed to open '%s': %s", path.c_str(), e.what());
    return nullptr;
  }

  auto info = std::make_unique<NFmiFastQueryInfo>(data.get());
  if (!info->IsGrid())
  {
    CPLError(CE_Failure, CPLE_NotSupported,
             "querydata driver only supports gridded data; '%s' uses station locations",
             path.c_str());
    return nullptr;
  }
  if (paramIdx >= info->SizeParams() || levelIdx >= info->SizeLevels())
  {
    CPLError(CE_Failure, CPLE_AppDefined,
             "Subdataset (param=%lu, level=%lu) out of range for '%s'",
             paramIdx, levelIdx, path.c_str());
    return nullptr;
  }

  const NFmiGrid* grid = info->Grid();
  if (grid == nullptr)
  {
    CPLError(CE_Failure, CPLE_AppDefined, "Missing grid descriptor in '%s'", path.c_str());
    return nullptr;
  }

  auto ds = std::make_unique<QDDataset>();
  ds->itsData = data;
  ds->itsInfo = std::move(info);
  ds->itsSourcePath = path;
  ds->itsParamIndex = paramIdx;
  ds->itsLevelIndex = levelIdx;
  ds->itsSubParamId = subParamId;
  ds->itsGridXSize = grid->XNumber();
  ds->itsGridYSize = grid->YNumber();
  ds->nRasterXSize = static_cast<int>(ds->itsGridXSize);
  ds->nRasterYSize = static_cast<int>(ds->itsGridYSize);

  // If a sub-param was requested, sanity-check that it actually exists in this
  // dataset; otherwise fail loudly rather than silently reading the parent.
  if (subParamId != 0)
  {
    if (!ds->itsInfo->Param(static_cast<FmiParameterName>(subParamId)) ||
        !ds->itsInfo->IsSubParamUsed())
    {
      CPLError(CE_Failure, CPLE_AppDefined,
               "Sub-parameter id %lu not found in '%s'",
               subParamId, path.c_str());
      return nullptr;
    }
  }

  ds->buildGeoTransformAndSRS();
  ds->buildBands();
  ds->itsRootGroup = buildRootGroup(*ds);
  // The dataset's description is shown as the file path in gdalinfo / QGIS. We
  // want it to point at the underlying .sqd; the subdataset spec lives in the
  // parent dataset's SUBDATASETS metadata, which is enough to round-trip.
  ds->SetDescription(path.c_str());
  ds->oOvManager.Initialize(ds.get(), path.c_str());
  return ds.release();
}

void QDDataset::buildSubdatasetList()
{
  CPLStringList md;
  int n = 1;

  auto emit = [&](unsigned long p, unsigned long l, unsigned long subId,
                  const std::string& desc) {
    const std::string nameKey = fmt::format("SUBDATASET_{}_NAME", n);
    const std::string descKey = fmt::format("SUBDATASET_{}_DESC", n);
    md.SetNameValue(nameKey.c_str(), formatSubdatasetSpec(itsSourcePath, p, l, subId).c_str());
    md.SetNameValue(descKey.c_str(), desc.c_str());
    ++n;
  };

  for (unsigned long p = 0; p < itsInfo->SizeParams(); ++p)
  {
    itsInfo->ParamIndex(p);
    const auto& dataIdent = itsInfo->Param();
    const std::string paramName = toUtf8(dataIdent.GetParamName().CharPtr());
    const long paramId = dataIdent.GetParamIdent();

    for (unsigned long l = 0; l < itsInfo->SizeLevels(); ++l)
    {
      itsInfo->LevelIndex(l);
      const NFmiLevel* level = itsInfo->Level();
      const float levelValue = (level != nullptr) ? level->LevelValue() : 0.f;
      const long levelTypeId = (level != nullptr) ? static_cast<long>(level->LevelType()) : 0;
      const std::string levelDesc =
          fmt::format("Level {} (type {})", levelValue, levelTypeId);

      // Composite parameter: emit one subdataset per sub-parameter rather than
      // a single one for the bit-packed parent (whose float values are the
      // raw packed encoding, useless to a generic GDAL consumer).
      if (dataIdent.HasDataParams() && dataIdent.GetDataParams() != nullptr)
      {
        for (const auto& subIdent : dataIdent.GetDataParams()->ParamsVector())
        {
          const long subId = subIdent.GetParamIdent();
          const std::string subName = toUtf8(subIdent.GetParamName().CharPtr());
          emit(p, l, static_cast<unsigned long>(subId),
               fmt::format("Param {} ({}) sub-of {} ({}), {}",
                           subId, subName, paramId, paramName, levelDesc));
        }
      }
      else
      {
        emit(p, l, 0,
             fmt::format("Param {} ({}), {}", paramId, paramName, levelDesc));
      }
    }
  }
  GDALDataset::SetMetadata(md.List(), "SUBDATASETS");
}

// Convert NFmiMetTime to epoch seconds (UTC).
static int64_t toEpochSeconds(const NFmiMetTime& tm)
{
  std::tm tt{};
  tt.tm_year = tm.GetYear() - 1900;
  tt.tm_mon = tm.GetMonth() - 1;
  tt.tm_mday = tm.GetDay();
  tt.tm_hour = tm.GetHour();
  tt.tm_min = tm.GetMin();
  tt.tm_sec = 0;
  return static_cast<int64_t>(timegm(&tt));
}

// Format an ISO-8601 string ("YYYY-MM-DDTHH:MM:SSZ") into the udunits style
// ("YYYY-MM-DD HH:MM:SS UTC") used in CF "time#units" metadata.
static std::string toUdunits(const std::string& iso)
{
  std::string out = iso;
  if (auto p = out.find('T'); p != std::string::npos) out[p] = ' ';
  if (auto p = out.rfind('Z'); p != std::string::npos) out.replace(p, 1, " UTC");
  return out;
}

void QDDataset::buildBands()
{
  // Position the iterator at the right (param, level). For sub-params we use
  // Param(FmiParameterName) which sets fUseSubParam=true; the parent index is
  // selected internally to match.
  if (itsSubParamId != 0)
    itsInfo->Param(static_cast<FmiParameterName>(itsSubParamId));
  else
    itsInfo->ParamIndex(itsParamIndex);
  itsInfo->LevelIndex(itsLevelIndex);

  // First pass: collect epoch seconds per timestep. Used both for per-band
  // numeric NETCDF_DIM_time values and for the dataset-level VALUES list.
  const unsigned long nT = itsInfo->SizeTimes();
  std::vector<int64_t> epochSecs(nT, 0);
  for (unsigned long t = 0; t < nT; ++t)
  {
    itsInfo->TimeIndex(t);
    epochSecs[t] = toEpochSeconds(itsInfo->Time());
  }
  // CF "hours since <origin>" — origin is the first timestep so the first
  // value is always 0 and the rest are integer hours for typical FMI forecast
  // data (hourly, 3-hourly, 6-hourly). Falls back to fractional hours for any
  // sub-hourly file (observational data) — udunits accepts that.
  const int64_t epoch0 = nT > 0 ? epochSecs[0] : 0;
  itsInfo->TimeIndex(0);
  const std::string originUdunits =
      nT > 0 ? toUdunits(formatTime(itsInfo->Time())) : std::string("1970-01-01 00:00:00 UTC");

  std::vector<std::string> hourValues(nT);
  for (unsigned long t = 0; t < nT; ++t)
  {
    const double hours = static_cast<double>(epochSecs[t] - epoch0) / 3600.0;
    hourValues[t] = fmt::format("{:g}", hours);  // "0", "3", "6.5", …
  }

  for (unsigned long t = 0; t < nT; ++t)
  {
    itsInfo->TimeIndex(t);
    auto* band = new QDRasterBand(this, static_cast<int>(t + 1), t);
    const std::string validTime = formatTime(itsInfo->Time());
    band->SetDescription(validTime.c_str());
    band->SetMetadataItem("VALID_TIME", validTime.c_str());
    band->SetMetadataItem("ORIGIN_TIME", formatTime(itsInfo->OriginTime()).c_str());
    // Per-band metadata that consumers' temporal controllers scan for:
    //   - "time": ISO-8601, used by QGIS Layer Properties → Temporal expressions
    //     via @band_description / @band_name fallbacks.
    //   - "NETCDF_DIM_time": NUMERIC offset matching the dataset-level
    //     "time#units" (CF/NetCDF convention). QGIS's "Calculate" button picks
    //     this up automatically when the dataset-level NETCDF_DIM_* metadata
    //     below is present.
    band->SetMetadataItem("time", validTime.c_str());
    band->SetMetadataItem("NETCDF_DIM_time", hourValues[t].c_str());
    SetBand(static_cast<int>(t + 1), band);
  }

  // Dataset-level CF/NetCDF temporal metadata. Lets QGIS's "Fixed Time Range
  // Per Band → Calculate" fill in per-band ranges automatically without the
  // user typing an expression, and makes xarray / cdo / ncview understand the
  // time axis in the same way they do native NetCDF.
  if (nT > 0)
  {
    GDALDataset::SetMetadataItem("NETCDF_DIM_EXTRA", "{time}");
    GDALDataset::SetMetadataItem(
        "NETCDF_DIM_time_DEF",
        fmt::format("{{{},6}}", nT).c_str());  // 6 = NC_DOUBLE in netCDF type codes
    std::string valuesList = "{";
    for (unsigned long i = 0; i < hourValues.size(); ++i)
    {
      if (i != 0) valuesList += ',';
      valuesList += hourValues[i];
    }
    valuesList += '}';
    GDALDataset::SetMetadataItem("NETCDF_DIM_time_VALUES", valuesList.c_str());
    GDALDataset::SetMetadataItem(
        "time#units", fmt::format("hours since {}", originUdunits).c_str());
    GDALDataset::SetMetadataItem("time#calendar", "gregorian");
  }

  // Dataset-level metadata describing the (param, level) we represent.
  const auto& dataIdent = itsInfo->Param();
  if (itsSubParamId != 0)
  {
    // dataIdent is the *parent* (combined) param; the sub-param's display name
    // has to be looked up in the parent's DataParams bag.
    std::string subName = fmt::format("subparam_{}", itsSubParamId);
    if (dataIdent.GetDataParams() != nullptr)
    {
      for (const auto& s : dataIdent.GetDataParams()->ParamsVector())
      {
        if (static_cast<unsigned long>(s.GetParamIdent()) == itsSubParamId)
        {
          subName = toUtf8(s.GetParamName().CharPtr());
          break;
        }
      }
    }
    GDALDataset::SetMetadataItem("PARAM_ID", fmt::format("{}", itsSubParamId).c_str());
    GDALDataset::SetMetadataItem("PARAM_NAME", subName.c_str());
    GDALDataset::SetMetadataItem("PARENT_PARAM_ID",
                                 fmt::format("{}", dataIdent.GetParamIdent()).c_str());
    GDALDataset::SetMetadataItem("PARENT_PARAM_NAME",
                                 toUtf8(dataIdent.GetParamName().CharPtr()).c_str());
  }
  else
  {
    GDALDataset::SetMetadataItem("PARAM_ID",
                                 fmt::format("{}", dataIdent.GetParamIdent()).c_str());
    GDALDataset::SetMetadataItem("PARAM_NAME", toUtf8(dataIdent.GetParamName().CharPtr()).c_str());
  }
  if (const NFmiLevel* lvl = itsInfo->Level())
  {
    GDALDataset::SetMetadataItem("LEVEL_VALUE", fmt::format("{}", lvl->LevelValue()).c_str());
    GDALDataset::SetMetadataItem(
        "LEVEL_TYPE", fmt::format("{}", static_cast<long>(lvl->LevelType())).c_str());
  }
}

void QDDataset::buildGeoTransformAndSRS()
{
  const NFmiArea* area = itsInfo->Area();
  if (area == nullptr) return;

  // SRS via WKT exposed by NFmiArea.
  itsSRS = std::make_unique<OGRSpatialReference>();
  itsSRS->SetAxisMappingStrategy(OAMS_TRADITIONAL_GIS_ORDER);
  const std::string wkt = area->WKT();
  if (!wkt.empty())
  {
    if (itsSRS->importFromWkt(wkt.c_str()) != OGRERR_NONE)
    {
      const std::string proj = area->ProjStr();
      if (!proj.empty()) itsSRS->importFromProj4(proj.c_str());
    }
  }

  // Geotransform from world rect (in projected coordinates, or degrees for latlon).
  // QD grids are stored bottom-up by default. We expose a top-down geotransform
  // and let QDRasterBand flip Y on read.
  //
  // NFmiRect uses *screen* Y convention internally: rect.Top() has the SMALLER
  // Y value (top of screen), rect.Bottom() has the LARGER Y value. So in the
  // projected coordinate system used by area->WorldRect(), rect.Top() maps to
  // the SOUTH edge (ymin) and rect.Bottom() maps to the NORTH edge (ymax) for
  // any area whose worldXY axis points northward (which is virtually all of
  // them — Equidistant Cylindrical, Mercator, LatLon, …).
  // Using min/max here keeps the result correct even if some exotic area
  // happens to encode the rect the other way around.
  const NFmiRect rect = area->WorldRect();
  const double left   = std::min(rect.Left(), rect.Right());
  const double right  = std::max(rect.Left(), rect.Right());
  const double southY = std::min(rect.Top(),  rect.Bottom());
  const double northY = std::max(rect.Top(),  rect.Bottom());

  if (itsGridXSize == 0 || itsGridYSize == 0) return;

  const double pixelW = (right  - left)   / static_cast<double>(itsGridXSize);
  const double pixelH = (northY - southY) / static_cast<double>(itsGridYSize);

  itsGeoTransform.xorig  = left;
  itsGeoTransform.xscale = pixelW;
  itsGeoTransform.xrot   = 0;
  itsGeoTransform.yorig  = northY;       // top of GDAL image = north
  itsGeoTransform.yrot   = 0;
  itsGeoTransform.yscale = -pixelH;      // negative = north-up
  itsHasGeoTransform = true;
}

CPLErr QDDataset::GetGeoTransform(GDALGeoTransform& gt) const
{
  if (!itsHasGeoTransform) return CE_Failure;
  gt = itsGeoTransform;
  return CE_None;
}

const OGRSpatialReference* QDDataset::GetSpatialRef() const
{
  return itsSRS.get();
}

}  // namespace GdalQueryData
}  // namespace SmartMet
