#pragma once

#include <gdal_priv.h>

namespace SmartMet
{
namespace GdalQueryData
{
// CreateCopy: write a GDAL source dataset out as a QueryData (.sqd) file. The
// source must be a regular grid (no rotation), have a usable SRS, and a single
// parameter (one band per time step). Returns a freshly opened read-only handle
// to the new file, or nullptr on failure (with a CPLError set).
//
// Creation options:
//   PARAM_ID         (default: from src PARAM_ID metadata, else 4 = TotalTemperature)
//   PARAM_NAME       (default: from src PARAM_NAME metadata, else "Data")
//   ORIGIN_TIME      ISO 8601 (default: from src ORIGIN_TIME, else first VALID_TIME)
//   LEVEL_VALUE      (default: 0)
//   LEVEL_TYPE       (default: 1 = ground/surface)
//   START_TIME       ISO 8601 — first band's valid time (default: from band metadata)
//   TIMESTEP_MINUTES interval between consecutive bands (default: from band metadata)
GDALDataset* createCopy(const char* pszFilename, GDALDataset* poSrcDS, int bStrict,
                        char** papszOptions, GDALProgressFunc pfnProgress, void* pProgressData);

}  // namespace GdalQueryData
}  // namespace SmartMet
